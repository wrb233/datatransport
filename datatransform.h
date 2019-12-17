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

	void timerTask();

	void writeSQLFileByInfo(QStringList &tableNameList,QStringList &sqlList,QString fileName=QDateTime::currentDateTime().toString("yyyyMMddhhmmss")+".sql",QString path="data");

	public slots:
	void update();



};
#endif 