#include "ToolUtil.h"
#include "datatransform.h"

extern Database *database;//实时数据库指针外部全局变量
extern DBPOOLHANDLE dbPoolHandle;//历史库外部全局变量
extern Logger hisRecordLog;//log4c日志
extern DataTransform* dataTransform;
extern  bool isActiveFault;
extern  QSettings *configIniRead;

AType AT_ampRating;
AType AT_BreakerType;
AType AT_ChangeNum;
AType AT_ChannelState;
AType AT_ChildrenList;
AType AT_CommitSyncEnd;
AType AT_ControlResult;
AType AT_DeviceProperty;
AType AT_DPCPointLink;
AType AT_DPSPointLink;
AType AT_Factory;
AType AT_FaultState;
AType AT_FeederLink;
AType AT_KeyName;
AType AT_Limit;
AType AT_MeasLink;
AType AT_MeasurementTypeLink;
AType AT_Model;
AType AT_MVPointLink;
AType AT_Name;
AType AT_OnlineStatus;
AType AT_OnlineTime;
AType AT_ParamMva;
AType AT_ParentLink;
AType AT_PSRLink;
AType AT_RTEnergized;
AType AT_State;
AType AT_Value;

OType OT_Analog;
OType OT_BSCPoint;
OType OT_DCSwitch;
OType OT_Disconnector;
OType OT_Discrete;
OType OT_DistributionTransformer;
OType OT_DMSCommunicateUnit;
OType OT_DPCPoint;
OType OT_DPSPoint;
OType OT_Feeder;
OType OT_IncrementCommit;
OType OT_LoadBreakSwitch;
OType OT_Machine;
OType OT_MVPoint;
OType OT_PMSBreaker;
OType OT_PMSBusbar;
OType OT_PMSDoubleWindingTransformer;
OType OT_PMSRoot;
OType OT_PMSStation;
OType OT_PMSTerminal;
OType OT_PMSThreeWindingTransformer;
OType OT_SubControlArea;
OType OT_TransformerWinding;


OType OT_PMSVLTP;
OType OT_Folder;



ToolUtil::ToolUtil():QObject()
{

}

ToolUtil::~ToolUtil()
{

}

bool ToolUtil::initOTypeAndAType()
{
	try
	{
		AT_ampRating = database->matchAType("ampRating");
		AT_BreakerType = database->matchAType("BreakerType");
		AT_ChangeNum = database->matchAType("ChangeNum");
		AT_ChannelState = database->matchAType("ChannelState");
		AT_ChildrenList = database->matchAType("ChildrenList");
		AT_CommitSyncEnd = database->matchAType("CommitSyncEnd");
		AT_ControlResult = database->matchAType("ControlResult");
		AT_DeviceProperty = database->matchAType("DeviceProperty");
		AT_DPCPointLink = database->matchAType("DPCPointLink");
		AT_DPSPointLink = database->matchAType("DPSPointLink");
		AT_Factory = database->matchAType("Factory");
		AT_FaultState = database->matchAType("FaultState");
		AT_FeederLink = database->matchAType("FeederLink");
		AT_KeyName = database->matchAType("KeyName");
		AT_Limit = database->matchAType("Limit");
		AT_MeasLink = database->matchAType("MeasLink");
		AT_MeasurementTypeLink = database->matchAType("MeasurementTypeLink");
		AT_Model = database->matchAType("Model");
		AT_MVPointLink = database->matchAType("MVPointLink");
		AT_Name = database->matchAType("Name");
		AT_OnlineStatus = database->matchAType("OnlineStatus");
		AT_OnlineTime = database->matchAType("OnlineTime");
		AT_ParamMva = database->matchAType("ParamMva");//额定视在功率AType
		AT_ParentLink = database->matchAType("ParentLink");
		AT_PSRLink = database->matchAType("PSRLink");
		AT_RTEnergized = database->matchAType("RTEnergized");//是否带电
		AT_State = database->matchAType("State");
		AT_Value = database->matchAType("Value");

		OT_Analog = database->matchOType("Analog");
		OT_BSCPoint = database->matchOType("BSCPoint");
		OT_DCSwitch = database->matchOType("DCSwitch");
		OT_Disconnector = database->matchOType("Disconnector");
		OT_Discrete = database->matchOType("Discrete");
		OT_DistributionTransformer = database->matchOType("DistributionTransformer");
		OT_DMSCommunicateUnit = database->matchOType("DMSCommunicateUnit");
		OT_DPCPoint = database->matchOType("DPCPoint");
		OT_DPSPoint = database->matchOType("DPSPoint");
		OT_Feeder = database->matchOType("Feeder");
		OT_IncrementCommit = database->matchOType("IncrementCommit");
		OT_LoadBreakSwitch = database->matchOType("LoadBreakSwitch");
		OT_Machine = database->matchOType("Machine");
		OT_MVPoint = database->matchOType("MVPoint");
		OT_PMSBreaker = database->matchOType("PMSBreaker");
		OT_PMSBusbar = database->matchOType("PMSBusbar");
		OT_PMSDoubleWindingTransformer = database->matchOType("PMSDoubleWindingTransformer");
		OT_PMSRoot = database->matchOType("PMSRoot");
		OT_PMSStation =  database->matchOType("PMSStation");
		OT_PMSTerminal = database->matchOType("PMSTerminal");
		OT_PMSThreeWindingTransformer = database->matchOType("PMSThreeWindingTransformer");
		OT_SubControlArea = database->matchOType("SubControlArea");
		OT_TransformerWinding = database->matchOType("TransformerWinding");



		OT_PMSVLTP = database->matchOType("PMSVLTP");
		OT_Folder = database->matchOType("Folder");

	}
	catch (Exception& e)
	{
		return false;
	}
	return true;
}

QString ToolUtil::getHisRecordConf(QString key)
{
	if(NULL==configIniRead)
	{
		char* cpsenv = getenv("CPS_ENV");
		QString cfgPath = QString::fromUtf8(cpsenv) + "/etc/" + "hisrecord.ini"; 
		QFile cfgFile(cfgPath);  
		if (!cfgFile.exists())
		{
			QSettings *configIniWrite = new QSettings(cfgPath, QSettings::IniFormat);   
			configIniWrite->setValue("/FaultTolerance/config", "on");
			configIniWrite->setValue("/InfoQueue/size", "1000000");
			configIniWrite->setValue("/Debug/config", "on");
			delete configIniWrite;
		}
		configIniRead = new QSettings(cfgPath, QSettings::IniFormat); 
	}

	QString returnValue = configIniRead->value(key).toString();  

	return returnValue;
}

//创建默认log4Cplus配置文件
void ToolUtil::createDefaultLogConfigFile(QString fileName,QString LoggerName)
{ 
	QStringList  content;  
	content.clear(); 
	content.append("log4cplus.rootLogger=ERROR, STDOUT"); 
	content.append("log4cplus.logger."+LoggerName +"=ERROR,R");  
	content.append("log4cplus.appender.STDOUT=log4cplus::ConsoleAppender");  
	content.append("log4cplus.appender.STDOUT.layout=log4cplus::PatternLayout");  
	content.append("log4cplus.appender.STDOUT.layout.ConversionPattern=%d{%m/%d/%y %H:%M:%S} [%t] %-5p %c{2} %%%x%% - %m [%l]%n");  

	content.append("log4cplus.appender.R=log4cplus::RollingFileAppender"); 
	char* cpsenv = getenv("CPS_ENV"); 
	QString logFilePath = QString::fromUtf8(cpsenv) + "/data/log/" + LoggerName + ".log";  
	content.append("log4cplus.appender.R.File = "+logFilePath);  

	content.append("log4cplus.appender.R.MaxFileSize=5000KB"); 
	content.append("log4cplus.appender.R.MaxBackupIndex=10");  
	content.append("log4cplus.appender.R.layout=log4cplus::PatternLayout"); 
	content.append("log4cplus.appender.R.layout.ConversionPattern=%d{%m/%d/%y %H:%M:%S} [%t] %-5p %c{2} %%%x%% - %m [%l]%n"); 

	QFile file(fileName);  
	if (file.open(QFile::WriteOnly | QFile::Text))   { 
		QTextStream out(&file);  
		for (int i = 0; i < content.size(); i++) {  
			out << content.at(i) << endl;  
		} 
	}  
} 

/************************************************************************/
/*      //连接历史库                                                                */
/************************************************************************/
bool ToolUtil::connectDB(int argc, char *argv[])
{
	std::string user,password,dataserver;

	for(int i=0;i<argc;i++)
	{
		if(qstricmp(argv[i],"-u") == 0)
		{
			while (*(argv[i+1])!='\0')
			{
				user += *(argv[i+1]++);
			}
		}

		if(qstricmp(argv[i],"-p") == 0)
		{
			while (*(argv[i+1])!='\0')
			{
				password += *(argv[i+1]++);
			}
		}

		if(qstricmp(argv[i],"-d") == 0)
		{
			while (*(argv[i+1])!='\0')
			{
				dataserver += *(argv[i+1]++);
			}
		}
	}

	if(user == "")
		user = "hds";
	if(password == "")
		password = "hds";
	if(dataserver == "")
		dataserver = "hstserver1+hstserver2";
	QStringList dbList = QString::fromStdString(dataserver).split("+");
	CPS_ORM_Initialize();
	DBPoolConfig cfg;
	cfg.m_bAutoCommit=true;
	cfg.m_nPoolSize=dbList.size();
	cfg.m_strUser=user;
	cfg.m_strPassword=password;	
	
	for(int i=0;i<dbList.size();i++)
	{
		cfg.m_strTNS[i] = dbList.at(i).toStdString();	
	}
	dbPoolHandle = CPS_ORM_NewDBPoolbyConfig(cfg);
	std::cout<<"connect to history database...";
	if(CPS_ORM_Connect(dbPoolHandle))
	{
		CPS_ORM_SetAutoCommit(dbPoolHandle,true);
		CPS_ORM_SetStreamAutoCommit(dbPoolHandle,true);
		return true;
	}
	return false;
}

/************************************************************************/
/*      //校验数据库连接，若失败重新连接历史库                                                                */
/************************************************************************/
bool ToolUtil::UpdateConnectDB()
{
	//检查数据库连接状态
	CPS_ORM_UpdatePool(dbPoolHandle);
	if(!CPS_ORM_GetDBPoolState(dbPoolHandle))
	{
		if(!CPS_ORM_Connect(dbPoolHandle))
		{
			CPS_ORM_SetAutoCommit(dbPoolHandle,true);
			CPS_ORM_SetStreamAutoCommit(dbPoolHandle,true);
			return false;
		}

		if(0==CPS_ORM_GetDBPoolState(dbPoolHandle))
		{
			LOG4CPLUS_ERROR(hisRecordLog, "Database dbPoolHandle State Error");
			return false;
		}
	}
	return true;
}

bool ToolUtil::isCurrentMachine()
{
	if ("on"==ToolUtil::getHisRecordConf("/FaultTolerance/config"))
	{
		if (!dataTransform->getFaultTolerance()->amITheCurrentMachine())
		{
			ToolUtil::myDebug("I am not TheCurrentMachine");
			LOG4CPLUS_ERROR(hisRecordLog, "I am not TheCurrentMachine");
			return false;
		}
	}	
	return true;
}

void ToolUtil::myDebug(QString content)
{
	if ("on"==ToolUtil::getHisRecordConf("/Debug/config"))
	{
		qDebug()<<content<< endl; ;
	}
}

bool ToolUtil::databaseRead(ObId obId, AType aType, Data* data)
{
	try
	{
		database->read(obId, aType, data);
	}
	catch (Exception& e)
	{
		ToolUtil::myDebug(QString::number(obId)+": DATABASE READ AT ERROR");
		return false;
	}
	return true;
}

bool ToolUtil::databaseReadTime(ObId obId, AType aType, Data* data, timespec_t &time)
{
	try
	{
		Request req;
		req.set(obId,aType,data,NULL,0,NORMAL_WRITE,&time);
		database->read(&req);
	}
	catch (Exception& e)
	{
		ToolUtil::myDebug(QString::number(obId)+": DATABASE READ Time ERROR");
		return false;
	}
	return true;
}

OType ToolUtil::databaseMatchOType(const char* name)
{
	OType oType = 0;
	try
	{
		oType = database->matchOType(name);
	}
	catch (Exception& e)
	{
		ToolUtil::myDebug(QString::fromUtf8(name)+": DATABASE MATCH OT ERROR");
	}
	return oType;
}

OType ToolUtil::databaseExtractOType(ObId obId)
{
	OType oType = 0;
	try
	{
		oType = database->extractOType(obId);
	}
	catch (Exception& e)
	{
		ToolUtil::myDebug(QString::number(obId)+": DATABASE Extract OT ERROR");
	}
	return oType;
}

int ToolUtil::databaseFind(OType oType, const Condition* conditions, int numberOfConditions,
	ObId* objects, int maximumNumberOfObjects)
{
	int num = 0;
	try
	{
		num = database->find(oType, conditions, numberOfConditions, objects, maximumNumberOfObjects);
	}
	catch (Exception& e)
	{
		ToolUtil::myDebug(QString::number(oType)+": DATABASE Find OT ERROR");
	}
	return num;
}

ObId ToolUtil::findOTypeByObId(ObId obId, OType toFindOType, OType stopOType)
{
	ObId toFindObId = 0;
	LinkData parentLinkData = 0;
	ToolUtil::databaseRead(obId, AT_ParentLink, &parentLinkData);
	if (parentLinkData!=0)
	{
		OType oType = ToolUtil::databaseExtractOType((ObId)parentLinkData);
		if (oType==toFindOType)
		{
			toFindObId = (ObId)parentLinkData;
		}else if (oType!=stopOType)
		{
			ObId tempObId = ToolUtil::findOTypeByObId((ObId)parentLinkData, toFindOType, stopOType);
			if (0!=tempObId)
			{
				toFindObId = tempObId;
			}
		}
	}
	return toFindObId;
}

QDateTime ToolUtil::convertTimespecToQDatetime(timespec_t ts)
{
	/*time_t t = ts.tv_sec;
	long tn = ts.tv_nsec;
	QDateTime timeTemp = QDateTime::fromTime_t(t);
	QString timeS = timeTemp.toString("yyyy-MM-dd hh:mm:ss");
	QString timeM = QString::number(tn/1000000);
	QString timeString = timeS + "." + timeM;
	QDateTime time = QDateTime::fromString(timeString, "yyyy-MM-dd hh:mm:ss.zzz");
	qDebug()<<time;
	qDebug()<<time.toString("yyyy-MM-dd hh:mm:ss.zzz");
	return time;*/
	int year, month, day, hour, minute, second, ms;
	OMSTime omT = OMSTime( ts );
	omT.getDateTimeFields( &year, &month, &day, &hour, &minute, &second, &ms );

	QTime q_tm = QTime( hour, minute, second, ms/1000 );
	QDate q_dt = QDate( year, month, day );
	QDateTime datetime = QDateTime( q_dt, q_tm );
	QString strtime = datetime.toString("yyyy-MM-dd hh:mm:ss.zzz");
	qDebug()<<strtime;

	return datetime;

}

QString ToolUtil::convertQMapToJson(QList<QMap<QString,QString> >list)
{
	QString json = "";

	QString jsonArrayStart = "[";
	QString jsonArrayEnd = "]";	
	QString jsonObjectStart = "{";
	QString jsonObjectEnd = "}";
	QString jsonObject = "\"%1\":\"%2\"";

	for (int i=0;i<list.size();i++)
	{
		QMap<QString,QString>::iterator it; 
		QString jsonTemp = "";
		for ( it = list[i].begin(); it != list[i].end(); ++it )
		{
			jsonTemp = jsonTemp.append(jsonObject.arg(it.key()).arg(it.value())).append(",");
		}
		jsonTemp = jsonTemp.left(jsonTemp.length()-1);
		json = json.append(jsonObjectStart).append(jsonTemp).append(jsonObjectEnd).append(",");
	}
	json = json.left(json.length()-1);	
	json = jsonArrayStart.append(json).append(jsonArrayEnd);
	
	return json;
}

void ToolUtil::writeJsonFileByInfo(QString json, QString fileName)
{
	ToolUtil::myDebug("MethodName:writeJsonFileByInfo start");
	char* cpsenv = getenv("CPS_ENV");
	QString sqlFileFolderPath = QDir::fromNativeSeparators(QString::fromUtf8(cpsenv)) + "/data/share/monitor/json/"; 
	QDir dir(sqlFileFolderPath);
	if(!dir.exists())
	{
		dir.mkpath(sqlFileFolderPath);
	}
	ToolUtil::myDebug(sqlFileFolderPath);

	fileName = fileName.append(QDateTime::currentDateTime().toString("_yyyyMMddhhmmsszzz"));
	fileName = fileName.append(".json");
 	QFile f(sqlFileFolderPath+fileName);  
	if(!f.open(QIODevice::WriteOnly | QIODevice::Text))  
	{  
		ToolUtil::myDebug("Open failed");
		return;
	}  

	QTextStream datOutput(&f); 

	datOutput.setCodec("UTF-8");
	datOutput << json << endl; 
	
	

	datOutput.flush();
	f.flush();
	f.close();
	ToolUtil::myDebug("MethodName:writeJsonFileByInfo end");
}