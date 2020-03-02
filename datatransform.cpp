#include "datatransform.h"
#include "IncrementCommit.h"
#include "NotificationCallback.h"
#include "ToolUtil.h"

extern  Database *database;
extern Logger datatransformLog;
extern  QMutex fileListMutex;

DataTransform::DataTransform(int &argc, char **argv,SignalHandler sigtermHandler,const OptionList& optionList,
	EnvironmentScope environmentScope) : QtApplication(argc, argv, sigtermHandler, optionList, environmentScope)
{
	database = getDatabase();
	//_database = createDatabase();

	if ("on"==ToolUtil::getIniConf("/FaultTolerance/config"))
	{
		//Ã‚Â½ÃƒÂ¸Ã‚Â³ÃƒÅ’ÃƒË†ÃƒÂÃ‚Â´ÃƒÂ­
		try
		{
			activateFaultTolerance();
		}catch(Exception& e)
		{
			ToolUtil::myDebug("Active faultTolerance error: system app exit");
			LOG4CPLUS_ERROR(datatransformLog, "Active faultTolerance error");
			std::exit(0);
		}
	}
}


//Ã‚Â±ÃƒÂ ÃƒÂÃ‚Â´Ã‚Â²Ãƒâ€ºÃ‚ÂºÃ‚Â¯ÃƒÅ ÃƒÂ½update()ÃƒÆ’Ã‚Â¿Ãƒâ€™Ã‚Â»ÃƒÆ’ÃƒÂ«Ãƒâ€“Ã‚Â´ÃƒÂÃƒÂÃƒâ€™Ã‚Â»Ã‚Â´ÃƒÅ½
void DataTransform::timerTask()
{
	update();//先执行一遍，然后利用QTimer定时器隔一段时间再执行	  
	QTimer *timer = new QTimer();
	connect(timer, SIGNAL(timeout()), this, SLOT(update()));
	timer->start(5000*60);
}

DataTransform::~DataTransform()
{

}



//update()Ã‚Â´ÃƒÂºÃƒâ€šÃƒÂ«Ãƒâ€šÃƒÅ¸Ã‚Â¼Ã‚Â­ffff水水水水
void DataTransform::update(){
	if (ToolUtil::getIniConf("/ConfigDataFunction/config")=="on")
	{
		handleDMSCommunicateUnit();    //Ãƒâ€°ÃƒÂºÃ‚Â³Ãƒâ€°Ãƒâ€“Ãƒâ€¢Ã‚Â¶Ãƒâ€¹Ã‚Â±ÃƒÂ­
		handleFeeder();                //Ãƒâ€°ÃƒÂºÃ‚Â³Ãƒâ€°Ãƒâ‚¬Ã‚Â¡ÃƒÂÃƒÅ¸Ã‚Â±ÃƒÂ­
		handlePMSStation();            //Ãƒâ€°ÃƒÂºÃ‚Â³Ãƒâ€°Ã‚Â±ÃƒÂ¤Ã‚ÂµÃƒÂ§Ãƒâ€¢Ã‚Â¾Ã‚Â±ÃƒÂ­
		handleDistributionTransformer(); //Ãƒâ€°ÃƒÂºÃ‚Â³Ãƒâ€°Ãƒâ€¦ÃƒÂ¤Ã‚Â±ÃƒÂ¤Ã‚Â±ÃƒÂ­
		handlePMSWindingTransformer();   //Ãƒâ€°ÃƒÂºÃ‚Â³Ãƒâ€°ÃƒÂÃ‚Â½Ã‚Â¾ÃƒÂ­Ã‚Â¡Ã‚Â¢ÃƒË†ÃƒÂ½Ã‚Â¾ÃƒÂ­Ã‚Â±ÃƒÂ¤Ãƒâ€˜Ã‚Â¹Ãƒâ€ ÃƒÂ·Ã‚Â±ÃƒÂ­  Ãƒâ€“ÃƒÂ·Ã‚Â±ÃƒÂ¤Ã‚Â±ÃƒÂ­
		handlePMSBusbar();               //Ãƒâ€°ÃƒÂºÃ‚Â³Ãƒâ€°Ãƒâ€žÃ‚Â¸ÃƒÂÃƒÅ¸Ã‚Â±ÃƒÂ­
		handlePMSKV();
		handleDataPointByConfig();
	}
	if (ToolUtil::getIniConf("/RuntimeDataFunction/config")=="on")
	{
		handleDataCollection();//处理上海华电系统采集数据
	}
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
		ToolUtil::writeJsonFileByInfo(json,"dmsterminal");
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
		ToolUtil::writeJsonFileByInfo(json,"feeder");
}

void DataTransform::handlePMSStation()
{
	OT_PMSStation = database->matchOType("PMSStation");
	Condition* conditions = NULL;
	int numberOfConditions = 0;
	int numElements = 0;
	numElements = database->find(OT_PMSStation,conditions,numberOfConditions);
	ObId* objects = new ObId[numElements];
	database->find(OT_PMSStation, conditions, numberOfConditions, objects, numElements);

	QList<QMap<QString,QString> >list;

	for( int i=0;i<numElements;i++)
	{

		QString PMSStation_id = QString::number(objects[i]); 

		StringData data;
        ToolUtil::databaseRead(objects[i], AT_Name, &data);			
		OMString str = (OMString)data;
		QString name = QString::fromUtf8(str.c_str());


		ObId PMSStationType_id = PMSStationType(objects[i]); 
		QString PMSStationTypeString_id = QString::number(PMSStationType_id);


		ObId PMSStationkvlevel_id = PMSStationkvlevel(objects[i]); 
		QString PMSStationkvlevelString_id = QString::number(PMSStationkvlevel_id);

		//QString PMSStationkvlevelString_id = PMSStationkvlevel(objects[i]);

		OType toFindOType = database->matchOType("SubControlArea");
		OType stopOType = database->matchOType("PMSRoot");
		ObId SubControlArea_id = ToolUtil::findOTypeByObId(objects[i], toFindOType, stopOType);
		QString SubControlAreaString_id = QString::number(SubControlArea_id);


		QMap<QString,QString> map;
		map.insert("id",PMSStation_id);
		map.insert("name",name);
		map.insert("kv_level",PMSStationkvlevelString_id);
		map.insert("type",PMSStationTypeString_id);
		map.insert("areaid",SubControlAreaString_id);

		list.append(map);
		
	}
		QString json = ToolUtil::convertQMapToJson(list);
		ToolUtil::writeJsonFileByInfo(json,"pmsstation");
}

void DataTransform::handleDistributionTransformer()
{
	OT_DistributionTransformer = database->matchOType("DistributionTransformer");
	Condition* conditions = NULL;
	int numberOfConditions = 0;
	int numElements = 0;
	numElements = database->find(OT_PMSStation,conditions,numberOfConditions);
	ObId* objects = new ObId[numElements];
	database->find(OT_DistributionTransformer, conditions, numberOfConditions, objects, numElements);

	QList<QMap<QString,QString> >list;

	for( int i=0;i<numElements;i++)
	{

		QString DistributionTransformer_id = QString::number(objects[i]); 

		StringData data;
        ToolUtil::databaseRead(objects[i], AT_Name, &data);			
		OMString str = (OMString)data;
		QString name = QString::fromUtf8(str.c_str());


		ObId VLTPLinkID = DistributionTransformerVLTPLink(objects[i]); 
		QString PMSVLTPLinkIDString = QString::number(VLTPLinkID);


		ObId FeederID = DistributionTransformerFeederID(objects[i]); 
		QString FeederIDString = QString::number(FeederID);


		OType toFindOType = database->matchOType("PMSStation");
		OType stopOType = database->matchOType("SubControlArea");
		ObId PMSStation_id = ToolUtil::findOTypeByObId(objects[i], toFindOType, stopOType);
		QString PMSStationString_id = QString::number(PMSStation_id);


		


		QMap<QString,QString> map;
		map.insert("id",DistributionTransformer_id);
		map.insert("dtname",name);
		map.insert("kv_level",PMSVLTPLinkIDString);
		map.insert("feeder_id",FeederIDString);
		map.insert("pmsstation_id",PMSStationString_id);

		
		

		list.append(map);
		
	}
		QString json = ToolUtil::convertQMapToJson(list);
		ToolUtil::writeJsonFileByInfo(json,"distributiontransformer");
}


void DataTransform::handlePMSWindingTransformer()
{
	OT_PMSDoubleWindingTransformer = database->matchOType("PMSDoubleWindingTransformer");
	OT_PMSThreeWindingTransformer = database->matchOType("PMSThreeWindingTransformer");
	Condition* conditions = NULL;
	int numberOfConditions = 0;
	int numElements1 = 0;
	int numElements2 = 0;
	int numElements = 0;
	numElements1 = database->find(OT_PMSDoubleWindingTransformer,conditions,numberOfConditions);
	numElements2 = database->find(OT_PMSThreeWindingTransformer,conditions,numberOfConditions);
	numElements = numElements1 + numElements2;

	ObId* objects = new ObId[numElements];


	database->find(OT_PMSDoubleWindingTransformer, conditions, numberOfConditions, objects, numElements1);

	QList<QMap<QString,QString> >list;

	for( int i=0;i<numElements1;i++)
	{

		QString PMSDoubleWindingTransformer_id = QString::number(objects[i]); 

		StringData data;
        ToolUtil::databaseRead(objects[i], AT_Name, &data);			
		OMString str = (OMString)data;
		QString name = QString::fromUtf8(str.c_str());


		ObId PMSWindingTransformerRatedCapa_data = PMSWindingTransformerRatedCapa(objects[i]); 
		QString PMSWindingTransformerRatedCapaString = QString::number(PMSWindingTransformerRatedCapa_data);

		OType toFindOType = database->matchOType("PMSStation");
		OType stopOType = database->matchOType("SubControlArea");
		ObId PMSStation_id = ToolUtil::findOTypeByObId(objects[i], toFindOType, stopOType);
		QString PMSStationString_id = QString::number(PMSStation_id);



		QMap<QString,QString> map;
		map.insert("id",PMSDoubleWindingTransformer_id);
		map.insert("name",name);
		map.insert("ratedcapacity",PMSWindingTransformerRatedCapaString);
		map.insert("objecttype","PMSDoubleWindingTransformer");
		map.insert("station_id",PMSStationString_id);
		

		list.append(map);
		
	}


	database->find(OT_PMSThreeWindingTransformer, conditions, numberOfConditions, objects, numElements2);

	//QList<QMap<QString,QString> >list;

	for( int j=0;j<numElements2;j++)
	{

		QString PMSThreeWindingTransformer_id = QString::number(objects[j]); 

		StringData data;
        ToolUtil::databaseRead(objects[j], AT_Name, &data);			
		OMString str = (OMString)data;
		QString name = QString::fromUtf8(str.c_str());

		ObId PMSWindingTransformerRatedCapa_data = PMSWindingTransformerRatedCapa(objects[j]); 
		QString PMSWindingTransformerRatedCapaString = QString::number(PMSWindingTransformerRatedCapa_data);
		
		OType toFindOType = database->matchOType("PMSStation");
		OType stopOType = database->matchOType("SubControlArea");
		ObId PMSStation_id = ToolUtil::findOTypeByObId(objects[j], toFindOType, stopOType);
		QString PMSStationString_id = QString::number(PMSStation_id);
		


		QMap<QString,QString> map;
		map.insert("id",PMSThreeWindingTransformer_id);
		map.insert("name",name);
		map.insert("ratedcapacity",PMSWindingTransformerRatedCapaString);
		map.insert("objecttype","PMSThreeWindingTransformer");
		map.insert("station_id",PMSStationString_id);

		list.append(map);
		
	}








	QString json = ToolUtil::convertQMapToJson(list);
	ToolUtil::writeJsonFileByInfo(json,"generatortransformer");
		
}


void DataTransform::handlePMSKV()
{
	OT_PMSVLTP = database->matchOType("PMSVLTP");

	OType OT_Folder = database->matchOType("Folder");          //找folder且name是VoltageGrade的
	const OMString folderName = "VoltageGrade";
	StringData obNameData(folderName);
	AType AT_Name = database->matchAType("Name");
	Condition conditions2;
	conditions2.set(AT_Name, EQ, (const Data*)(&obNameData));
	
	
	Condition* conditions1 = NULL;
	int numberOfConditions = 0;
	int numElements1 = 0;
	int numElements = 0;
	int numElements2 = 0;

	numElements1 = database->find(OT_PMSVLTP,conditions1,numberOfConditions);

	numElements2 = database->find(OT_Folder, &conditions2, 1);

	numElements = numElements1 + numElements2;

	ObId* objects = new ObId[numElements];


	database->find(OT_PMSVLTP, conditions1, numberOfConditions, objects, numElements1);
	
	
	QList<QMap<QString,QString> >list;

	for( int i=0;i<numElements1;i++)
	{

		QString PMSKV_id = QString::number(objects[i]); 

		StringData data;
        ToolUtil::databaseRead(objects[i], AT_Name, &data);			
		OMString str = (OMString)data;
		QString name = QString::fromUtf8(str.c_str());


		
		double kv_value = PMSKVvalue(name.toUtf8().data());

		QString kv_valueString_id = QString::number(kv_value);

		





		
		


		QMap<QString,QString> map;
		map.insert("id",PMSKV_id);
		map.insert("name",name);
		map.insert("value",kv_valueString_id);
		
		
		

		list.append(map);
		
	}

	
	database->find(OT_Folder, &conditions2, 1, objects, numElements2);
	for( int j=0;j<numElements2;j++)
	{

		 


		/*

		StringData data;
        ToolUtil::databaseRead(objects[j], AT_Name, &data);			
		OMString str = (OMString)data;
		QString name = QString::fromUtf8(str.c_str());
		*/
		ContainerData data;
		ToolUtil::databaseRead(objects[j], AT_ChildrenList, &data);

		int numberOfVoltageGrade = data.getNumberOfElements();

		//ObId*  obIDs = new ObId[numberOfVoltageGrade];

		const ObId* VoltageGradeObid = data.getObIds();



		for(int k=0;k<numberOfVoltageGrade;k++)
		{
		  
		  QString VoltageGrade_id = QString::number(VoltageGradeObid[k]);


		  StringData data;
          ToolUtil::databaseRead(VoltageGradeObid[k], AT_Name, &data);			
		  OMString str = (OMString)data;
		  QString name = QString::fromUtf8(str.c_str());


		  double kv_value = PMSKVvalue(name.toUtf8().data());

		QString kv_valueString_id = QString::number(kv_value);


		  QMap<QString,QString> map;
		  map.insert("id",VoltageGrade_id);
		  map.insert("name",name);
		  map.insert("value",kv_valueString_id);
		
		  list.append(map);

		}

		
		


		
		
	}
	





		QString json = ToolUtil::convertQMapToJson(list);
		ToolUtil::writeJsonFileByInfo(json,"pmskv");
}


void DataTransform::handlePMSBusbar()
{
	OT_PMSBusbar = database->matchOType("PMSBusbar");
	Condition* conditions = NULL;
	int numberOfConditions = 0;
	int numElements = 0;
	numElements = database->find(OT_PMSBusbar,conditions,numberOfConditions);
	ObId* objects = new ObId[numElements];
	database->find(OT_PMSBusbar, conditions, numberOfConditions, objects, numElements);

	QList<QMap<QString,QString> >list;

	for( int i=0;i<numElements;i++)
	{

		QString PMSBusbar_id = QString::number(objects[i]); 

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
		map.insert("id",PMSBusbar_id);
		map.insert("name",name);
		map.insert("kv_id",kv_levelString);
		map.insert("station_id",PMSStationString_id);
		
		

		list.append(map);
		
	}
		QString json = ToolUtil::convertQMapToJson(list);
		ToolUtil::writeJsonFileByInfo(json,"bus");
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

int DataTransform::PMSStationType(ObId dmsObjId) //get DMSCommunicateUnit Type
{
	AType at_PMSStationType;
	try
	{
		at_PMSStationType= database->matchAType("substationType");
	}
	catch(Exception& e)
	{
		return -1;
	}
	ChoiceData typeData;
	try
	{
		database->read(dmsObjId,at_PMSStationType,&typeData);
	}
	catch(Exception& e)
	{
		return -1;
	}
	return (int)typeData;
}

ObId DataTransform::PMSStationkvlevel(ObId dmsObjId) //get DMSCommunicateUnit Type
{
	AType at_PMSStationkvlevel;
	try
	{
		at_PMSStationkvlevel= database->matchAType("VoltageGrade");
		
	}
	catch(Exception& e)
	{
		return -1;
	}
	ChoiceData typeData;
	try
	{
		database->read(dmsObjId,at_PMSStationkvlevel,&typeData);


		const OMString *VoltageGradeValue = typeData.getStrings();            //getStrings()
		OMString strVoltageGradeValue = (OMString)*VoltageGradeValue;
		QString VoltageGradeValueString = QString::fromUtf8(strVoltageGradeValue.c_str());
		//qDebug()<<VoltageGradeValueString;

		

		
	OType OT_Folder = database->matchOType("Folder");          //找folder且name是VoltageGrade的
	const OMString folderName = "VoltageGrade";
	StringData obNameData(folderName);
	AType AT_Name = database->matchAType("Name");
	Condition conditions2;
	conditions2.set(AT_Name, EQ, (const Data*)(&obNameData));
	int numElements2 = database->find(OT_Folder, &conditions2, 1);
	ObId* objects = new ObId[numElements2];
	database->find(OT_Folder, &conditions2, 1, objects, numElements2);
		/*
		OT_PMSVLTP = database->matchOType("PMSVLTP");
		Condition* conditions = NULL;
		int numberOfConditions = 0;
		int numElements = 0;
		numElements = database->find(OT_PMSVLTP,conditions,numberOfConditions);
		ObId* objects = new ObId[numElements];
		database->find(OT_PMSVLTP, conditions, numberOfConditions, objects, numElements);
		
		*/
	
		for( int j=0;j<numElements2;j++)
	{

		 


		/*

		StringData data;
        ToolUtil::databaseRead(objects[j], AT_Name, &data);			
		OMString str = (OMString)data;
		QString name = QString::fromUtf8(str.c_str());
		*/
		ContainerData data;
		ToolUtil::databaseRead(objects[j], AT_ChildrenList, &data);

		int numberOfVoltageGrade = data.getNumberOfElements();

		//ObId*  obIDs = new ObId[numberOfVoltageGrade];

		const ObId* VoltageGradeObid = data.getObIds();



		for(int k=0;k<numberOfVoltageGrade;k++)
		{
		  
		  ObId VoltageGrade_id = VoltageGradeObid[k];


		  StringData data;
          ToolUtil::databaseRead(VoltageGradeObid[k], AT_Name, &data);			
		  OMString str = (OMString)data;
		  QString name = QString::fromUtf8(str.c_str());

		  if(VoltageGradeValueString == name){

					return VoltageGrade_id;

				}else{

					ObId VoltageGrade_id_fourhundred = 51539608382;

					return VoltageGrade_id_fourhundred;
				}
		 


		  

		}

		
		


		
		
	}
		
				

				
		
	

		
		

	}
	catch(Exception& e)
	{
		return -1;
	}
	

}

ObId DataTransform::DistributionTransformerVLTPLink(ObId dmsObjId)    //get Subordinate object,return obid
{
	AType at_VLTPLink;
	try
	{
		at_VLTPLink = database->matchAType("VLTPLink");
	}
	catch(Exception& e)
	{
		ToolUtil::myDebug(QString::number(dmsObjId)+": DATABASE Extract OT ERROR");
		return 0;
	}
	LinkData VLTPLink;
	try
	{
		database->read(dmsObjId,at_VLTPLink,&VLTPLink);
	}
	catch(Exception& e)
	{
		ToolUtil::myDebug(QString::number(dmsObjId)+": DATABASE Extract OT ERROR");
		return 0;
	}
	return (ObId)VLTPLink;
}

ObId DataTransform::DistributionTransformerFeederID(ObId dmsObjId)    //get Subordinate object,return obid
{
	AType at_FeederID;
	try
	{
		at_FeederID= database->matchAType("FeederID");
	}
	catch(Exception& e)
	{
		ToolUtil::myDebug(QString::number(dmsObjId)+": DATABASE Extract OT ERROR");
		return 0;
	}
	LongLongData FeederID;
	try
	{
		database->read(dmsObjId,at_FeederID,&FeederID);
	}
	catch(Exception& e)
	{
		ToolUtil::myDebug(QString::number(dmsObjId)+": DATABASE Extract OT ERROR");
		return 0;
	}
	return (ObId)FeederID;
}



int DataTransform::PMSWindingTransformerRatedCapa(ObId dmsObjId)    //get Subordinate object,return obid
{
	AType at_RatedCapa;
	try
	{
		at_RatedCapa= database->matchAType("RatedCapa");
	}
	catch(Exception& e)
	{
		ToolUtil::myDebug(QString::number(dmsObjId)+": DATABASE Extract OT ERROR");
		return 0;
	}
	ChoiceData RatedCapa;
	try
	{
		database->read(dmsObjId,at_RatedCapa,&RatedCapa);
	}
	catch(Exception& e)
	{
		ToolUtil::myDebug(QString::number(dmsObjId)+": DATABASE Extract OT ERROR");
		return 0;
	}
	return (int)RatedCapa;
}



double DataTransform::PMSKVvalue(char a[])    //get Subordinate object,return obid
{
	
    double d=0;
    int i,t=0;
    for (i=0;a[i]>='.'&& a[i]<='A';i++)
       {
        if(a[i]=='.')
              {
                t = 10;
               }
         else
           {
              if(t==0)
              {
                 d = d*10 + (a[i]-'0');
                  }
               else
                  {
                  d = d + (double)(a[i]-'0')/t;
                    t *=10;
                }
            }
        }
return d;

	
 
   
 
     
     
     
}




//Ã‚Â³ÃƒÂµÃƒÅ Ã‚Â¼Ã‚Â»Ã‚Â¯Ãƒâ€”Ã‚Â¢Ã‚Â²ÃƒÂ¡Ã‚Â»ÃƒËœÃ‚ÂµÃƒÂ·Ã‚ÂºÃ‚Â¯ÃƒÅ ÃƒÂ½
void DataTransform::initNotificationCallback()
{
	//Ã‚Â¼ÃƒÂ ÃƒÅ’ÃƒÂ½Ãƒâ€¦ÃƒÂ¤Ã‚ÂµÃƒÂ§Ãƒâ€“Ãƒâ€¢Ã‚Â¶Ãƒâ€¹Ã‚ÂµÃƒâ€žÃƒâ€ÃƒÅ¡ÃƒÂÃƒÅ¸Ãƒâ€”Ã‚Â´ÃƒÅ’Ã‚Â¬
	Request reqOnline;
	reqOnline.set(AT_OnlineStatus, OT_DMSCommunicateUnit, NEW_NOTIFICATION, notificationOnlineStatus, (void*)this,NOTIFY_ON_CHANGE); 
	database->notify(&reqOnline, 1);

	//Ã‚Â¼ÃƒÂ ÃƒÅ’ÃƒÂ½Ãƒâ€¦ÃƒÂ¤Ã‚ÂµÃƒÂ§Ãƒâ€“Ãƒâ€¢Ã‚Â¶Ãƒâ€¹Ã‚ÂµÃƒâ€žÃƒÂÃ‚Â¨Ã‚ÂµÃƒâ‚¬Ãƒâ€”Ã‚Â´ÃƒÅ’Ã‚Â¬
	Request reqChannel;
	reqChannel.set(AT_ChannelState, OT_DMSCommunicateUnit, NEW_NOTIFICATION, notificationChannelState, (void*)this,NOTIFY_ON_CHANGE); 
	database->notify(&reqChannel, 1);

	//Ã‚Â¼ÃƒÂ ÃƒÅ’ÃƒÂ½Ãƒâ€™Ã‚Â£Ã‚Â¿ÃƒËœÃ‚ÂµÃƒâ€žÃ‚Â¿ÃƒËœÃƒâ€“Ãƒâ€ Ã‚Â½ÃƒÂ¡Ã‚Â¹ÃƒÂ»ÃƒÅ ÃƒÂ´ÃƒÂÃƒâ€
	Request reqControlResult;
	reqControlResult.set(AT_ControlResult, OT_DPCPoint, NEW_NOTIFICATION, notificationDPCAction, (void*)this,NOTIFY_ON_WRITE); 
	database->notify(&reqControlResult, 1);

	//Ã‚Â¼ÃƒÂ ÃƒÅ’ÃƒÂ½Ãƒâ€™Ã‚Â£Ã‚Â²ÃƒÂ¢Ã‚ÂµÃƒâ€žvalueÃƒâ€“Ã‚Âµ
	Request reqValue;
	reqValue.set(AT_Value, OT_MVPoint, NEW_NOTIFICATION, notificationMVPointValue, (void*)this,NOTIFY_ON_CHANGE); 
	database->notify(&reqValue, 1);

	//Ã‚Â¼ÃƒÂ ÃƒÅ’ÃƒÂ½Ãƒâ€ÃƒÂ¶ÃƒÂÃ‚Â¿Ã‚Â¸ÃƒÂ¼ÃƒÂÃƒâ€š
	IncrementCommit ic;
	ic.NotfyIncrementCommit();
}

void DataTransform::writeSQLFileByInfo(QStringList &tableNameList,QStringList &sqlList, QString fileName,QString path)
{
	ToolUtil::myDebug("MethodName:writeSQLFileByInfo start");
	char* cpsenv = getenv("CPS_ENV");
	QString sqlFileFolderPath = QDir::fromNativeSeparators(QString::fromUtf8(cpsenv)) + "/data/datatransform/"+path+"/"; 
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
void DataTransform::handleDataCollection()//处理上海华电系统采集数据
{
	char* cpsenv = getenv("CPS_ENV");
	QString sqlFileFolderPath = QDir::fromNativeSeparators(QString::fromUtf8(cpsenv)) + "/data/share/"; 
	QString filePath = sqlFileFolderPath + "datapoint.txt";
	QStringList  dataList;
	ToolUtil::readFileByPath(filePath, dataList);

	QList<QMap<QString,QString> >list;
	foreach(QString data, dataList)
	{
		QMap<QString,QString> map;

		ObId obId = (ObId)data.toLongLong();
		
		if (ToolUtil::databaseExtractOType(obId)==OT_DPSPoint)
		{
			map.insert("obid",QString::number(obId));

			StringData nameData = "";
			ToolUtil::databaseRead(obId, AT_Name, &nameData);
			std::string name = std::string(nameData);
			map.insert("des",QString::fromUtf8(name.c_str()));
			map.insert("type",QString::number(0));

			IntegerData stateDataTemp;
			ToolUtil::databaseRead(obId, AT_FieldState, &stateDataTemp);
			int stateData = (int)stateDataTemp;
			map.insert("value",QString::number(stateData));
		}else if (ToolUtil::databaseExtractOType(obId)==OT_MVPoint)
		{
			map.insert("obid",QString::number(obId));

			StringData nameData = "";
			ToolUtil::databaseRead(obId, AT_Name, &nameData);
			std::string name = std::string(nameData);
			map.insert("des",QString::fromUtf8(name.c_str()));
			map.insert("type",QString::number(1));

			FloatData valueDataTemp;
			ToolUtil::databaseRead(obId, AT_FieldValue, &valueDataTemp);
			float valueData = (float)valueDataTemp;
			map.insert("value",QString::number(valueData));
		}
		if (map.size()>0)
		{
			list.append(map);	
		}		
	}
	QString json = ToolUtil::convertQMapToJson(list);
	ToolUtil::writeJsonFileByInfo(json,"data");
}

void DataTransform::handleDataPointByConfig()
{
	char* cpsenv = getenv("CPS_ENV");
	QString fileFolderPath = QDir::fromNativeSeparators(QString::fromUtf8(cpsenv)) + "/data/share/"; 
	QString filePath = fileFolderPath + "point_config.json";
	QStringList  dataList;
	ToolUtil::readFileByPath(filePath, dataList);
	QString data = dataList.join("");
	if (!data.isEmpty())
	{
		cJSON* json=cJSON_Parse(data.toStdString().c_str());	
		cJSON *arrayItem = cJSON_GetObjectItem(json,"data");
		QList<QMap<QString,QString> >list;

		for (int i=0;i<cJSON_GetArraySize(arrayItem);i++)
		{
			QMap<QString,QString> map;

			cJSON *object = cJSON_GetArrayItem(arrayItem,i);		
			cJSON *tablenameItem = cJSON_GetObjectItem(object,"tablename");
			map.insert("tablename",QString::fromUtf8(tablenameItem->valuestring));
			cJSON *fieldnameItem = cJSON_GetObjectItem(object,"fieldname");
			map.insert("fieldname",QString::fromUtf8(fieldnameItem->valuestring));
			cJSON *objidItem = cJSON_GetObjectItem(object,"objid");
			map.insert("objid",QString::fromUtf8(objidItem->valuestring));
			cJSON *atypeItem = cJSON_GetObjectItem(object,"atype");
			map.insert("atype",QString::fromUtf8(atypeItem->valuestring));
			ObId obId = (ObId)QString::fromUtf8(objidItem->valuestring).toLongLong();
			AType atype = ToolUtil::databaseMatchAType(atypeItem->valuestring);
			if (atype==AT_Value)
			{
				FloatData floatData;
				ToolUtil::databaseRead(obId,atype,&floatData);
				double valueD = (double)floatData;
				map.insert("value",QString::number(valueD));
			}else if (atype==AT_State)
			{
				IntegerData intData;
				ToolUtil::databaseRead(obId,atype,&intData);
				int valueI = (int)intData;
				map.insert("value",QString::number(valueI));
			}	

			list.append(map);	
		}
		QString jsonOut = ToolUtil::convertQMapToJson(list);
		ToolUtil::writeJsonFileByInfo(jsonOut,"point_config");

		cJSON_Delete(json);
	}
}																				