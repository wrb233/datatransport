#ifndef HISRECORD_H
#define HISRECORD_H

#include "common.h"
#include "cJSON.h"				  

class DataTransform: public QtApplication
{
	Q_OBJECT
public:

	//构造函数
	DataTransform(int &argc, char **argv,
		SignalHandler sigtermHandler,
		const OptionList& optionList,
		EnvironmentScope environmentScope);

	//析构函数
	~DataTransform();

	//初始化注册回调函数
	void initNotificationCallback();

	ObId getFeederLink(ObId dmsObjId);

	int DeviceProperty(ObId dmsObjId);

	void timerTask();

	void writeSQLFileByInfo(QStringList &tableNameList,QStringList &sqlList,QString fileName=QDateTime::currentDateTime().toString("yyyyMMddhhmmss")+".sql",QString path="data");

	void handleDMSCommunicateUnit();
	void handleFeeder();


	ObId getkvlevel(ObId dmsObjId);

	void handlePMSStation();
	int PMSStationType(ObId dmsObjId);
	ObId PMSStationkvlevel(ObId dmsObjId);


	void handleDistributionTransformer();

	ObId DistributionTransformerVLTPLink(ObId dmsObjId);
	ObId DistributionTransformerFeederID(ObId dmsObjId);

	void handlePMSWindingTransformer();
	//void handlePMSDoubleWindingTransformer();
	//void handlePMSThreeWindingTransformer();


	int PMSWindingTransformerRatedCapa(ObId dmsObjId);

	void handlePMSBusbar();


	void handlePMSKV();

	void handleDataCollection();//处理上海华电系统采集数据															   
	void handleDataPointByConfig();//处理宝鸡手机app数据点，根据配置文件point_config.json读取																										
	double PMSKVvalue(char a[]);
	

	public slots:

	void update();



};
#endif 