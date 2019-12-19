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
DBPOOLHANDLE dbPoolHandle;//������ʷ��ȫ�ֱ���
Database *database = NULL;//ʵʱ���ݿ�
Logger hisRecordLog = Logger::getInstance(LOG4CPLUS_TEXT("hisRecordLog"));//ȫ����־
QMutex infoCacheMutex;//��Ϣ��������ȫ������
QList<DataModel> infoList;
QMutex fileListMutex;//�ļ��б�����ȫ������
QMutex initMutex;//��ʼ���ڴ�����ȫ������
QSettings *configIniRead = NULL;//Ӧ�ü������ļ���Ϣ

QMap<ObId, Terminal> terminals;
QMap<ObId, DPCPoint> dpcPoints;
QMap<ObId, Feeder> feeders;
QMap<ObId, MVPointOfFeeder> mvPointOfFeeders;

//�Զ���ȽϺ���
int maxnum(int num1, int  num2)
{
	int result;
    if (num1 > num2)
    	result = num1;
    else
        result = num2;
    return result;
}

//������ObId���ֵ�ObId���ֵĺ����������Ķ�
void ObjectIdtoName(ObId obId)
	{
    StringData data;//����StringData��������
    ToolUtil::databaseRead(obId, AT_Name, &data);
	std::string s1 = data;
	//qDebug()<<"the name of obId is:"<<QString::fromStdString(s1)<<endl;
	qDebug()<<"the name of obId is:"<<QString::fromStdString(s1)<<endl;
	}

//��ȡ�����¼�ObId
void GetAllSubordinateOrgCode(ObId obId)
{	
	ContainerData ChildrenListOfParent;//����ContainerData��������
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
	//��ӡ�汾��Ϣ

	
	qDebug("------------------------------------");
	int num1 = 3;
	int	num2 = 6;
	int result;
    result = maxnum(num1, num2);  // ���ñȽϺ���maxnum()
    qDebug() << "max vlaue is" << result << endl;
	QString str1="welcome";
	qDebug()<<str1;


	if (echoVersion(argc, argv)) 
	{
		return 0;
	}

	//���ô�ӡ��־

	
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
	

	//����datatransportӦ�ü�app
	OptionList optionList;
	dataTransform = new DataTransform(argc, argv, NULL,optionList,RUNTIME_SCOPE);
	
	//��ʼ��OType��AType
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
		at_DeviceProperty= g_pDatabase->matchAType("DeviceProperty");     //�豸����
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
	
	



		//���ĸ�outFile����Ч
	    //outFile <<"the name of obId is:"<<QString::fromStdString(s1)<<endl;
	    //outFile << QString::fromStdString(s1) <<endl;
		//outFile << data << endl;
		//outFile <<ObjectIdtoName(objects[i]);
	//}


	




	//qDebug()<<"----------------------------------------------";

	

	/*

	StringData data;//����StringData��������
    ToolUtil::databaseRead(240518168577, AT_Name, &data);
	std::string s1 = data;
	qDebug()<<"the name of PMSRoot is:"<<QString::fromStdString(s1);


	ObjectIdtoName(240518168577);
	
	
    //����PMSRoot��ObjectId�ҵ�PMSRoot��ChildrenList����
	ContainerData childrenListOfPMSRoot;//����ContainerData��������
    ToolUtil::databaseRead(240518168577, AT_ChildrenList, &childrenListOfPMSRoot);
	std::string s2 = childrenListOfPMSRoot;
	qDebug()<<"the ChildrenList of PMSRoot is:"<<QString::fromStdString(s2);


	GetAllSubordinateOrgCode(240518168577);



	//ContainerData childrenListOfParent[10];//����ContainerData��������


	ContainerData childrenListOfbaoji;//����ContainerData��������
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

	//��ȡ��ʷ��ָ��
	if (!ToolUtil::connectDB(argc, argv))
	{
		ToolUtil::myDebug("connect hstdatabase failed");
		LOG4CPLUS_ERROR(hisRecordLog, "connect hstdatabase failed");
		std::exit(0);
	}

	//�����ʼ����Ϣ����Ϣ����,��֪�߳�A����״̬��ĳ�ʼ������
	DataModel dm;
	dm.enumKey=Init_Table;
	infoList.append(dm);

	//��ʼ���ص�����
	dataTransform->initNotificationCallback();

	//�������̶߳�ʱ������Ϣ����
	InfoHandleThread* infoHandleThread = new InfoHandleThread();
	infoHandleThread->start();

	//�������̶߳�ʱ���������ļ����洢�����ݿ���
	DBThread* dbThread = new DBThread();
	dbThread->start();	


	*/


	dataTransform->svc();
	return 1;
}