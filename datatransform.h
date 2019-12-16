#ifndef HISRECORD_H
#define HISRECORD_H

#include "common.h"

class DataTransform: public QtApplication
{
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

	void writeSQLFileByInfo(QStringList &tableNameList,QStringList &sqlList,QString fileName=QDateTime::currentDateTime().toString("yyyyMMddhhmmss")+".sql",QString path="data");
};
#endif 