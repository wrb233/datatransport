#include "datatransform.h"
#include "version.h"
#include "ToolUtil.h"
#include "InfoHandleThread.h"
#include "DBThread.h"
#include "InfoEnum.h"
#include "DataModel.h"

#include<string>
#include<iostream>
#include<typeinfo>
#include<fstream>

using namespace std; 

DataTransform* dataTransform = NULL;
DBPOOLHANDLE dbPoolHandle;//定义历史库全局变量
Database *database = NULL;//实时数据库
Logger hisRecordLog = Logger::getInstance(LOG4CPLUS_TEXT("hisRecordLog"));//全局日志
QMutex infoCacheMutex;//消息缓存锁（全局锁）
QList<DataModel> infoList;
QMutex fileListMutex;//文件列表锁（全局锁）
QMutex initMutex;//初始化内存锁（全局锁）
QSettings *configIniRead = NULL;//应用级配置文件信息

QMap<ObId, Terminal> terminals;
QMap<ObId, DPCPoint> dpcPoints;
QMap<ObId, Feeder> feeders;
QMap<ObId, MVPointOfFeeder> mvPointOfFeeders;

//自定义比较函数
int maxnum(int num1, int  num2)
{
	int result;
    if (num1 > num2)
    	result = num1;
    else
        result = num2;
    return result;
}

//创建由ObId数字到ObId名字的函数，方便阅读
void ObjectIdtoName(ObId obId)
	{
    StringData data;//声明StringData数据类型
    ToolUtil::databaseRead(obId, AT_Name, &data);
	std::string s1 = data;
	//qDebug()<<"the name of obId is:"<<QString::fromStdString(s1)<<endl;
	qDebug()<<"the name of obId is:"<<QString::fromStdString(s1)<<endl;
	}

//获取所有下级ObId
void GetAllSubordinateOrgCode(ObId obId)
{	
	ContainerData ChildrenListOfParent;//声明ContainerData数据类型
	ToolUtil::databaseRead(obId, AT_ChildrenList, &ChildrenListOfParent);
	int numofcon =  ChildrenListOfParent.getNumberOfElements();
	const ObId *temparr =  new ObId[numofcon];	
	temparr = ChildrenListOfParent.getObIds();
	for( int i=0,j=0;i<numofcon;i++,j++ ){
		
		//printf("the subordinates of this obId:%s are showed as fllows:","obId") ;
		cout << temparr[i] << endl;

		cout << typeid(temparr[i]).name() << endl;
		
		//printf(ObjectIdtoName((char *)temparr[i]));
		ObjectIdtoName(temparr[i]);
	}
	
}




int main(int argc, char *argv[])
{
	//打印版本信息

	
	qDebug("------------------------------------");
	int num1 = 3;
	int	num2 = 6;
	int result;
    result = maxnum(num1, num2);  // 调用比较函数maxnum()
    qDebug() << "max vlaue is" << result << endl;
	QString str1="welcome";
	qDebug()<<str1;


	if (echoVersion(argc, argv)) 
	{
		return 0;
	}

	//配置打印日志

	
	QString processName = "hisRecordLog";
	char* cpsenv = getenv("CPS_ENV");
	QString cfgPath = QString::fromUtf8(cpsenv) + "/etc/logConfig/" + processName + ".properties"; 
	QFile cfgFile(cfgPath);  
	if (!cfgFile.exists()) 
	{    
		ToolUtil::createDefaultLogConfigFile(cfgPath, "hisRecordLog"); 
	} 

	log4cplus::initialize();  
	LogLog::getLogLog()->setInternalDebugging(true); 
	Logger root = Logger::getRoot(); 
	ConfigureAndWatchThread configureThread(cfgPath.toStdString().c_str(), 5 * 1000);
	

	//定义datatransport应用级app
	OptionList optionList;
	dataTransform = new DataTransform(argc, argv, NULL,optionList,RUNTIME_SCOPE);
	
	//初始化OType和AType
	if (!ToolUtil::initOTypeAndAType())
	{
		ToolUtil::myDebug("Init OType And AType error");
		LOG4CPLUS_ERROR(hisRecordLog, "Init OType And AType error");
		std::exit(0);
	}

	dataTransform->timerTask();





ChoiceData MainWindow::DeviceProperty(ObId dmsObjId) //get DMSCommunicateUnit Type
{
	AType at_DeviceProperty;
	try
	{
		at_DeviceProperty= g_pDatabase->matchAType("DeviceProperty");     //设备属性
	}
	catch(Exception& e)
	{
		ToolUtil::myDebug(QString::number(obId)+": DATABASE Extract OT ERROR");
		return -1;
	}
	ChoiceData typeData;
	try
	{
		g_pDatabase->read(id,at_DeviceProperty,&typeData);
	}
	catch(Exception& e)
	{
		ToolUtil::myDebug(QString::number(obId)+": DATABASE Extract OT ERROR");
		return -1;
	}
	return (int)typeData;
}
	
	



		//这四个outFile不生效
	    //outFile <<"the name of obId is:"<<QString::fromStdString(s1)<<endl;
	    //outFile << QString::fromStdString(s1) <<endl;
		//outFile << data << endl;
		//outFile <<ObjectIdtoName(objects[i]);
	//}


	




	//qDebug()<<"----------------------------------------------";

	

	/*

	StringData data;//声明StringData数据类型
    ToolUtil::databaseRead(240518168577, AT_Name, &data);
	std::string s1 = data;
	qDebug()<<"the name of PMSRoot is:"<<QString::fromStdString(s1);


	ObjectIdtoName(240518168577);
	
	
    //根据PMSRoot的ObjectId找到PMSRoot的ChildrenList宝鸡
	ContainerData childrenListOfPMSRoot;//声明ContainerData数据类型
    ToolUtil::databaseRead(240518168577, AT_ChildrenList, &childrenListOfPMSRoot);
	std::string s2 = childrenListOfPMSRoot;
	qDebug()<<"the ChildrenList of PMSRoot is:"<<QString::fromStdString(s2);


	GetAllSubordinateOrgCode(240518168577);



	//ContainerData childrenListOfParent[10];//声明ContainerData数组类型


	ContainerData childrenListOfbaoji;//声明ContainerData数据类型
    ToolUtil::databaseRead(1133871366145, AT_ChildrenList, &childrenListOfbaoji);	
	std::string s3 = childrenListOfbaoji;
	qDebug()<<"the ChildrenList of baoji is:"<<QString::fromStdString(s3);

	GetAllSubordinateOrgCode(1133871366145);

	GetAllSubordinateOrgCode(1133871366150);
	GetAllSubordinateOrgCode(1133871366148);
	GetAllSubordinateOrgCode(1133871366147);
	GetAllSubordinateOrgCode(1133871366152);
	GetAllSubordinateOrgCode(1133871366149);
	GetAllSubordinateOrgCode(1133871366151);
	GetAllSubordinateOrgCode(1133871366146);
	
	*/



	//GetAllSubordinateOrgCode(temparr[i]);

	//int i;
    //ToolUtil::databaseRead(240518168577, AT_ChildrenList, &childrenListOfParent[i]);
	/*std::string s3 = childrenListOfPMSRoot;
	qDebug()<<"the ChildrenList of PMSRoot is:"<<QString::fromStdString(s3);*/

	/*

	//获取历史库指针
	if (!ToolUtil::connectDB(argc, argv))
	{
		ToolUtil::myDebug("connect hstdatabase failed");
		LOG4CPLUS_ERROR(hisRecordLog, "connect hstdatabase failed");
		std::exit(0);
	}

	//插入初始化信息到消息队列,告知线程A进行状态表的初始化工作
	DataModel dm;
	dm.enumKey=Init_Table;
	infoList.append(dm);

	//初始化回调函数
	dataTransform->initNotificationCallback();

	//启动新线程定时处理消息队列
	InfoHandleThread* infoHandleThread = new InfoHandleThread();
	infoHandleThread->start();

	//启动新线程定时处理数据文件，存储到数据库中
	DBThread* dbThread = new DBThread();
	dbThread->start();	


	*/


	dataTransform->svc();
	return 1;
}