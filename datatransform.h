#ifndef HISRECORD_H
#define HISRECORD_H

#include "common.h"

class DataTransform: public QtApplication
{
	Q_OBJECT
public:

	//���캯��
	DataTransform(int &argc, char **argv,
		SignalHandler sigtermHandler,
		const OptionList& optionList,
		EnvironmentScope environmentScope);

	//��������
	~DataTransform();

	//��ʼ��ע��ص�����
	void initNotificationCallback();

	ObId getFeederLink(ObId dmsObjId);

	int DeviceProperty(ObId dmsObjId);

	void timerTask();

	void writeSQLFileByInfo(QStringList &tableNameList,QStringList &sqlList,QString fileName=QDateTime::currentDateTime().toString("yyyyMMddhhmmss")+".sql",QString path="data");

	void handleDMSCommunicateUnit();
	void handleFeeder();


	ObId getkvlevel(ObId dmsObjId);
	

	public slots:
	void update();



};
#endif 