#include "DBThread.h"
#include "ToolUtil.h"
#include "datatransform.h"

extern DataTransform* dataTransform;
extern  QMutex fileListMutex;
extern DBPOOLHANDLE dbPoolHandle;

DBThread::DBThread(QObject *parent): QThread(parent)
{
	qDebug() << "DB Thread : " << QThread::currentThreadId();
}

DBThread::~DBThread()
{

}

void DBThread::run()  
{
	qDebug() << "DB Run Thread : " << QThread::currentThreadId();
	while (true)
	{
		msleep(5000);
		qDebug() << "DB Thread is Working: " << QThread::currentThreadId();

		char* cpsenv = getenv("CPS_ENV");
		QString sqlFileFolderPath = QDir::fromNativeSeparators(QString::fromUtf8(cpsenv)) + "/data/datatransform/data/"; 
		QDir dir(sqlFileFolderPath);
		if(!dir.exists())
		{
			continue;
		}
		QStringList filters;
		filters<<QString("*.sql");
		dir.setFilter(QDir::Files | QDir::NoSymLinks);
		dir.setNameFilters(filters);
		
		if ( dir.count()>0)
		{
			QFileInfoList qFileInfoList;
			{
				QMutexLocker locker(&fileListMutex);
				qFileInfoList = dir.entryInfoList();
			}
			
			qDebug()<<qFileInfoList.size();
			for (int i=0;i<qFileInfoList.size();i++)
			{
				QFileInfo qFileInfo = qFileInfoList.at(i);
				qDebug()<<qFileInfo.absoluteFilePath();
				QString filePath = qFileInfo.absoluteFilePath();
				QStringList sqlList;

				if (readSQLFile(filePath,sqlList))
				{
					QStringList badSQLList;
					QStringList badTableList;
					foreach(QString sqlOrTableName,sqlList)
					{
						if (sqlOrTableName.startsWith("INSERT")||sqlOrTableName.startsWith("UPDATE")||sqlOrTableName.startsWith("DELETE"))
						{
							if (!executeSQL(sqlOrTableName))
							{
								badSQLList.append(sqlOrTableName);
							}
						}else{
							if (!checkTable(sqlOrTableName))
							{
								 badTableList.append(sqlOrTableName);
							}
						}
					}
					if (badTableList.size()>0||badSQLList.size()>0)
					{
						QString fileName=QDateTime::currentDateTime().toString("yyyyMMddhhmmss")+"_bad.sql";
						dataTransform->writeSQLFileByInfo(badTableList, badSQLList, fileName, "error");//将异常sql写入到error文件夹下
					}
					
					QFile::remove(filePath);//删除文件
					//qFileInfo.dir().remove(qFileInfo.fileName());
				}
			}		
		}
	}
}

bool DBThread::readSQLFile(QString &filePath,QStringList &sqlList)
{
	QFile file(filePath);  
	if(file.open(QIODevice::ReadOnly | QIODevice::Text)) 
	{
		QTextStream in(&file);  

		QString line = in.readLine();  

		while (!line.isNull()) 
		{
			sqlList.append(line);
			line = in.readLine();
		}
		/*while(!file.atEnd())
		{  
		QByteArray line = file.readLine();  
		QString str(line);  
		sqlList.append(str);
		}   */
		in.flush();
		file.flush();
		file.close();
		return true;
	}  
	return false;
}

bool DBThread::checkTable(QString tableName)
{
	if (ToolUtil::UpdateConnectDB())
	{
		int tableState = CPS_ORM_ExistTable(dbPoolHandle, tableName.trimmed().toStdString());
		if (0==tableState)//如果表不存在，则新建表
		{
			if (createTable(tableName))
			{
				if (createPrimaryKey(tableName))
				{
					return true;
				}
			}
		}else if (1==tableState)//表存在
		{
			return true;
		}
	}
	return false;
}

bool DBThread::createTable(QString tableName)
{
	ORMColumnDescVector m_descVector;
	if(tableName=="H_TERMINAL_RUNTIME")
	{
		setColoum(m_descVector, "SERVICEID", "bigint(19)", 0, 1);
		setColoum(m_descVector, "OBID", "bigint(19)", 0, 1, 1);
		setColoum(m_descVector, "ONLINESTAT", "short");
		setColoum(m_descVector, "ONLINETIME", "timestamp");
		setColoum(m_descVector, "WORKSTAT", "short");
		setColoum(m_descVector, "WORKTIME", "timestamp");
		setColoum(m_descVector, "TYPE", "short");
		setColoum(m_descVector, "MODEL", "char(1024)");
		setColoum(m_descVector, "FACTORY", "char(2048)");
		setColoum(m_descVector, "FEEDER", "bigint(19)");
		setColoum(m_descVector, "STATION", "bigint(19)");
		setColoum(m_descVector, "AREA", "bigint(19)");
		
	}else if(tableName=="H_FEEDER_RUNTIME")
	{
		setColoum(m_descVector, "SERVICEID", "bigint(19)", 0, 1);
		setColoum(m_descVector, "OBID", "bigint(19)", 0, 1, 1);
		setColoum(m_descVector, "AMPRATING", "double");
		setColoum(m_descVector, "LOADSTAT", "short");
		setColoum(m_descVector, "LOADTIME", "timestamp");
		setColoum(m_descVector, "MVPOINT", "bigint(19)");
		setColoum(m_descVector, "STATION", "bigint(19)");
		setColoum(m_descVector, "AREA", "bigint(19)");

	}else if (tableName.startsWith("H_TERMINAL_RECORD_"))
	{
		setColoum(m_descVector, "SERVICEID", "bigint(19)", 0, 1);
		setColoum(m_descVector, "OBID", "bigint(19)", 0, 1);
		setColoum(m_descVector, "ONLINESTAT", "short");
		setColoum(m_descVector, "ONLINETIME", "timestamp", 0, 1);
		setColoum(m_descVector, "TYPE", "short");
		setColoum(m_descVector, "MODEL", "char(1024)");
		setColoum(m_descVector, "FACTORY", "char(2048)");
		setColoum(m_descVector, "FEEDER", "bigint(19)");
		setColoum(m_descVector, "STATION", "bigint(19)");
		setColoum(m_descVector, "AREA", "bigint(19)");
	}else if (tableName.startsWith("H_DPCPOINT_RECORD_"))
	{
		setColoum(m_descVector, "SERVICEID", "bigint(19)", 0, 1);
		setColoum(m_descVector, "OBID", "bigint(19)", 0, 1);
		setColoum(m_descVector, "DPCACTION", "short");
		setColoum(m_descVector, "ACTIONTIME", "timestamp", 0, 1);
		setColoum(m_descVector, "ACTIONRESULT", "short");
		setColoum(m_descVector, "STATION", "bigint(19)");
		setColoum(m_descVector, "AREA", "bigint(19)");
	}else if (tableName.startsWith("H_FEEDER_RECORD_"))
	{
		setColoum(m_descVector, "SERVICEID", "bigint(19)", 0, 1);
		setColoum(m_descVector, "OBID", "bigint(19)", 0, 1);
		setColoum(m_descVector, "LOADSTAT", "short");
		setColoum(m_descVector, "LOADTIME", "timestamp", 0, 1);
		setColoum(m_descVector, "STATION", "bigint(19)");
		setColoum(m_descVector, "AREA", "bigint(19)");
	}
	if(CPS_ORM_CreateTable(dbPoolHandle, tableName.toStdString(), m_descVector)==1)
	{
		ToolUtil::myDebug("CreateTable:"+tableName);
		return true;
	} 
	return false;
}

void DBThread::setColoum(ORMColumnDescVector &m_descVector, std::string m_strName, std::string m_strType, bool m_bNullOK, bool m_bIndexed, bool m_bKey, bool m_bUnique)
{
	ORMColumnDesc columnDesc;
	columnDesc.m_strName = m_strName;
	columnDesc.m_strType = m_strType;
	columnDesc.m_bNullOK = m_bNullOK;
	columnDesc.m_bIndexed = m_bIndexed;
	columnDesc.m_bKey = m_bKey;
	columnDesc.m_bUnique = m_bUnique;
	m_descVector.push_back(columnDesc);
}

bool DBThread::createPrimaryKey(QString tableName)
{
	msleep(1000);
	// 增加主键
	ACE_Time_Value tv = ACE_OS::gettimeofday();
	timespec_t st = tv;
	char tmp[512];
	sprintf(tmp, "ind_%lld_%ld", st.tv_sec, st.tv_nsec);
	std::string indexName = tmp;

	QString sqlTemplate = "alter table %1 add constraint %2 primary key";
	QString sql = sqlTemplate.arg(tableName).arg(QString(indexName.c_str()));
	
	if(tableName=="H_TERMINAL_RUNTIME")
	{
		//do nothing
		return true;
	}else if(tableName=="H_FEEDER_RUNTIME")
	{
		//do nothing
		return true;
	}else if(tableName.startsWith("H_TERMINAL_RECORD_"))
	{
		sql = sql + "(OBID,ONLINETIME)";

	}else if (tableName.startsWith("H_DPCPOINT_RECORD_"))
	{
		sql = sql + "(OBID,ACTIONTIME)";
	}else if (tableName.startsWith("H_FEEDER_RECORD_"))
	{
		sql = sql + "(OBID,LOADTIME)";
	}

	if(-1==CPS_ORM_DirectExecuteSql(dbPoolHandle, sql.toUtf8().data(), 0))
	{
		ToolUtil::myDebug("CreateTablePrimaryKey error:"+tableName);
		return false;
	}
	return true;
}

bool DBThread::executeSQL(QString sql)
{
	try
	{
		if (ToolUtil::UpdateConnectDB())
		{
			if(CPS_ORM_DirectExecuteSql(dbPoolHandle,sql.toUtf8().data())>=0)
			{
				ToolUtil::myDebug(sql);
				ToolUtil::myDebug("INSERT  DATA SUCCESS!");
				return true;
			}
		}
	}
	catch (Exception& e)
	{
		ToolUtil::myDebug("INSERT  DATA FAILURE Exception!");
	}
	
	return false;
}