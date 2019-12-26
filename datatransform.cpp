#include "datatransform.h"
#include "IncrementCommit.h"
#include "NotificationCallback.h"
#include "ToolUtil.h"

extern  Database *database;
extern Logger hisRecordLog;
extern  QMutex fileListMutex;

DataTransform::DataTransform(int &argc, char **argv,SignalHandler sigtermHandler,const OptionList& optionList,
	EnvironmentScope environmentScope) : QtApplication(argc, argv, sigtermHandler, optionList, environmentScope)
{
	database = getDatabase();
	//_database = createDatabase();

	if ("on"==ToolUtil::getHisRecordConf("/FaultTolerance/config"))
	{
		//进程容错
		try
		{
			activateFaultTolerance();
		}catch(Exception& e)
		{
			ToolUtil::myDebug("Active faultTolerance error: system app exit");
			LOG4CPLUS_ERROR(hisRecordLog, "Active faultTolerance error");
			std::exit(0);
		}
	}
}


//编写槽函数update()每一秒执行一次
void DataTransform::timerTask()
{
	QTimer *timer = new QTimer();
	connect(timer, SIGNAL(timeout()), this, SLOT(update()));
	timer->start(1000*60);
}

DataTransform::~DataTransform()
{

}



//update()代码逻辑
void DataTransform::update(){

	handleDMSCommunicateUnit();    //生成终端表
	handleFeeder();                //生成馈线表
}


void DataTransform::handleDMSCommunicateUnit()
{
	OT_DMSCommunicateUnit = database->matchOType("DMSCommunicateUnit");
	Condition* conditions = NULL;
	int numberOfConditions = 0;
	int numElements = 0;
	numElements = database->find(OT_DMSCommunicateUnit,conditions,numberOfConditions);
	ObId* objects = new ObId[numElements];
	database->find(OT_DMSCommunicateUnit, conditions, numberOfConditions, objects, numElements);
	
	QList<QMap<QString,QString> >list;

	for( int i=0;i<numElements;i++)
	{
		QString DMSCommunicateUnit_id = QString::number(objects[i]); 
		
		StringData data;
        ToolUtil::databaseRead(objects[i], AT_Name, &data);
		OMString str = (OMString)data;
		QString name = QString::fromUtf8(str.c_str());

		ObId DeviceProperty_id = DeviceProperty(objects[i]); 
		QString DevicePropertyString_id = QString::number(DeviceProperty_id);

		ObId Feeder_id = getFeederLink(objects[i]); 
		QString FeederString_id = QString::number(Feeder_id);

		QMap<QString,QString> map;
		map.insert("id",DMSCommunicateUnit_id);
		map.insert("terminal_name",name);
		map.insert("terminal_type",DevicePropertyString_id);
		map.insert("source_id",FeederString_id);

		list.append(map);		
	}
		QString json = ToolUtil::convertQMapToJson(list);
		ToolUtil::writeJsonFileByInfo(json,"DMSTerminal");
}

void DataTransform::handleFeeder()
{
	OT_Feeder = database->matchOType("Feeder");
	Condition* conditions = NULL;
	int numberOfConditions = 0;
	int numElements = 0;
	numElements = database->find(OT_Feeder,conditions,numberOfConditions);
	ObId* objects = new ObId[numElements];
	database->find(OT_Feeder, conditions, numberOfConditions, objects, numElements);

	QList<QMap<QString,QString> >list;

	for( int i=0;i<numElements;i++)
	{

		QString Feeder_id = QString::number(objects[i]); 

		StringData data;
        ToolUtil::databaseRead(objects[i], AT_Name, &data);			
		OMString str = (OMString)data;
		QString name = QString::fromUtf8(str.c_str());


		ObId kv_level = getkvlevel(objects[i]); 
		QString kv_levelString = QString::number(kv_level);



		OType toFindOType = database->matchOType("PMSStation");
		OType stopOType = database->matchOType("SubControlArea");
		ObId PMSStation_id = ToolUtil::findOTypeByObId(objects[i], toFindOType, stopOType);
		QString PMSStationString_id = QString::number(PMSStation_id);
		
		QMap<QString,QString> map;
		map.insert("id",Feeder_id);
		map.insert("feedername",name);
		map.insert("kv_level",kv_levelString);
		map.insert("pmsstation_id",PMSStationString_id);

		list.append(map);
		
	}
		QString json = ToolUtil::convertQMapToJson(list);
		ToolUtil::writeJsonFileByInfo(json,"Feeder");
}

ObId DataTransform::getFeederLink(ObId dmsObjId)    //get Subordinate object,return obid
{
	AType at_feedrLink;
	try
	{
		at_feedrLink= database->matchAType("FeederLink");
	}
	catch(Exception& e)
	{
		ToolUtil::myDebug(QString::number(dmsObjId)+": DATABASE Extract OT ERROR");
		return 0;
	}
	LinkData feederLnk;
	try
	{
		database->read(dmsObjId,at_feedrLink,&feederLnk);
	}
	catch(Exception& e)
	{
		ToolUtil::myDebug(QString::number(dmsObjId)+": DATABASE Extract OT ERROR");
		return 0;
	}
	return (ObId)feederLnk;
}

int DataTransform::DeviceProperty(ObId dmsObjId) //get DMSCommunicateUnit Type
{
	AType at_DeviceProperty;
	try
	{
		at_DeviceProperty= database->matchAType("DeviceProperty");
	}
	catch(Exception& e)
	{
		return -1;
	}
	ChoiceData typeData;
	try
	{
		database->read(dmsObjId,at_DeviceProperty,&typeData);
	}
	catch(Exception& e)
	{
		return -1;
	}
	return (int)typeData;
}

ObId DataTransform::getkvlevel(ObId dmsObjId)    //get Subordinate object,return obid
{
	AType at_kvlevel;
	try
	{
		at_kvlevel= database->matchAType("VLTPLink");
	}
	catch(Exception& e)
	{
		ToolUtil::myDebug(QString::number(dmsObjId)+": DATABASE Extract OT ERROR");
		return 0;
	}
	LinkData kvlevel;
	try
	{
		database->read(dmsObjId,at_kvlevel,&kvlevel);
	}
	catch(Exception& e)
	{
		ToolUtil::myDebug(QString::number(dmsObjId)+": DATABASE Extract OT ERROR");
		return 0;
	}
	return (ObId)kvlevel;
}




//初始化注册回调函数
void DataTransform::initNotificationCallback()
{
	//监听配电终端的在线状态
	Request reqOnline;
	reqOnline.set(AT_OnlineStatus, OT_DMSCommunicateUnit, NEW_NOTIFICATION, notificationOnlineStatus, (void*)this,NOTIFY_ON_CHANGE); 
	database->notify(&reqOnline, 1);

	//监听配电终端的通道状态
	Request reqChannel;
	reqChannel.set(AT_ChannelState, OT_DMSCommunicateUnit, NEW_NOTIFICATION, notificationChannelState, (void*)this,NOTIFY_ON_CHANGE); 
	database->notify(&reqChannel, 1);

	//监听遥控的控制结果属性
	Request reqControlResult;
	reqControlResult.set(AT_ControlResult, OT_DPCPoint, NEW_NOTIFICATION, notificationDPCAction, (void*)this,NOTIFY_ON_WRITE); 
	database->notify(&reqControlResult, 1);

	//监听遥测的value值
	Request reqValue;
	reqValue.set(AT_Value, OT_MVPoint, NEW_NOTIFICATION, notificationMVPointValue, (void*)this,NOTIFY_ON_CHANGE); 
	database->notify(&reqValue, 1);

	//监听增量更新
	IncrementCommit ic;
	ic.NotfyIncrementCommit();
}

void DataTransform::writeSQLFileByInfo(QStringList &tableNameList,QStringList &sqlList, QString fileName,QString path)
{
	ToolUtil::myDebug("MethodName:writeSQLFileByInfo start");
	char* cpsenv = getenv("CPS_ENV");
	QString sqlFileFolderPath = QDir::fromNativeSeparators(QString::fromUtf8(cpsenv)) + "/data/hisrecord/"+path+"/"; 
	QDir dir(sqlFileFolderPath);
	if(!dir.exists())
	{
		dir.mkpath(sqlFileFolderPath);
	}
	ToolUtil::myDebug(sqlFileFolderPath);

	QMutexLocker locker(&fileListMutex);
	QFile f(sqlFileFolderPath+fileName);  
	if(!f.open(QIODevice::WriteOnly | QIODevice::Text))  
	{  
		ToolUtil::myDebug("Open failed");
		return;
	}  

	QTextStream datOutput(&f); 
	foreach(QString tableName,tableNameList)
	{
		datOutput << tableName<< endl; 
	}

	foreach(QString sql,sqlList)
	{
		datOutput << sql<< endl; 
	}

	datOutput.flush();
	f.flush();
	f.close();
	ToolUtil::myDebug("MethodName:writeSQLFileByInfo end");
}