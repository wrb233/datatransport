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
		//�����ݴ�
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


//��д�ۺ���update()ÿһ��ִ��һ��
void DataTransform::timerTask()
{
	QTimer *timer = new QTimer();
	connect(timer, SIGNAL(timeout()), this, SLOT(update()));
	timer->start(1000*60);
}

DataTransform::~DataTransform()
{

}



//update()�����߼�
void DataTransform::update()




{
	



	  
	
	//�½������
	//ofstream outFile;
    //���ļ�,��D�̸�Ŀ¼�´���Test2.txt�ļ�
    //outFile.open("D:\\Test2.txt");
	//��ʼ��find������Ҫ�Ĳ���


	OT_DMSCommunicateUnit = database->matchOType("DMSCommunicateUnit");
	Condition* conditions = NULL;
	int numberOfConditions = 0;
	int numElements = 0;
	numElements = database->find(OT_DMSCommunicateUnit,conditions,numberOfConditions);
	//�½�*��ObId����
	ObId* objects = new ObId[numElements];
	
	//find��������
	database->find(OT_DMSCommunicateUnit, conditions, numberOfConditions, objects, numElements);
	
	
	//������������ObId������� 2282��
	//outFile << "OTypeΪDMSCommunicateUnit�Ľڵ㹲��" <<numElements << "��" << endl;

	
	//����forѭ��ȡ������ObId
	//QStringList jsonList;

	QList<QMap<QString,QString>>list;
	for( int i=0;i<numElements;i++)
	{

		//�������ObId
		//outFile << objects[i] << endl;

		//����StringData��������
		StringData data;
		//����databaseRead�����õ����ص�name����
        ToolUtil::databaseRead(objects[i], AT_Name, &data);
		
		ObId tmpId = getFeederLink(objects[i]); ///feeder id
		
		QString id = QString::number(objects[i]);

		OMString str = (OMString)data;
		QString name = QString::fromUtf8(str.c_str());

		

		
			QMap<QString,QString> map;
			map.insert("id",id);
			map.insert("name",name);
			list.append(map);
		
		

		
		//jsonList.append(json);	
	
		//string name = data;

		//�������ObId��Ӧ��name����
		//outFile << name <<endl;
		
	}

		QString json = ToolUtil::convertQMapToJson(list);
		ToolUtil::writeJsonFileByInfo(json,"H_TEST");
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

//��ʼ��ע��ص�����
void DataTransform::initNotificationCallback()
{
	//��������ն˵�����״̬
	Request reqOnline;
	reqOnline.set(AT_OnlineStatus, OT_DMSCommunicateUnit, NEW_NOTIFICATION, notificationOnlineStatus, (void*)this,NOTIFY_ON_CHANGE); 
	database->notify(&reqOnline, 1);

	//��������ն˵�ͨ��״̬
	Request reqChannel;
	reqChannel.set(AT_ChannelState, OT_DMSCommunicateUnit, NEW_NOTIFICATION, notificationChannelState, (void*)this,NOTIFY_ON_CHANGE); 
	database->notify(&reqChannel, 1);

	//����ң�صĿ��ƽ������
	Request reqControlResult;
	reqControlResult.set(AT_ControlResult, OT_DPCPoint, NEW_NOTIFICATION, notificationDPCAction, (void*)this,NOTIFY_ON_WRITE); 
	database->notify(&reqControlResult, 1);

	//����ң���valueֵ
	Request reqValue;
	reqValue.set(AT_Value, OT_MVPoint, NEW_NOTIFICATION, notificationMVPointValue, (void*)this,NOTIFY_ON_CHANGE); 
	database->notify(&reqValue, 1);

	//������������
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