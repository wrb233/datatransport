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

void DataTransform::timerTask()
{
	QTimer *timer = new QTimer();
	connect(timer, SIGNAL(timeout()), this, SLOT(update()));
	timer->start(1000);
}

DataTransform::~DataTransform()
{

}

void DataTransform::update()
{
	//新建输出流
	ofstream outFile;
    //打开文件,在D盘根目录下创建Test2.txt文件
    outFile.open("D:\\Test2.txt");
	//初始化find函数需要的参数
	OT_DMSCommunicateUnit = database->matchOType("DMSCommunicateUnit");
	Condition* conditions = NULL;
	int numberOfConditions = 0;
	int numElements = 0;
	numElements = database->find(OT_DMSCommunicateUnit,conditions,numberOfConditions);
	//新建*个ObId对象
	ObId* objects = new ObId[numElements];
	
	//find函数重载
	database->find(OT_DMSCommunicateUnit, conditions, numberOfConditions, objects, numElements);
	
	
	//把满足条件的ObId个数输出
	outFile << "OType为DMSCommunicateUnit的节点共有" <<numElements << "个" << endl;

	
	//定义for循环取出所有ObId
	for( int i=0;i<numElements;i++)
	{

		//输出所有ObId
		outFile << objects[i] << endl;

		//声明StringData数据类型
		StringData data;
		//利用databaseRead函数得到返回的name属性
        ToolUtil::databaseRead(objects[i], AT_Name, &data);
		//string name = data;

		//输出所有ObId对应的name属性
		//outFile << name <<endl;
	}
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