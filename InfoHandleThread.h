#ifndef INFOHANDLETHREAD_H
#define INFOHANDLETHREAD_H

#include "common.h"
#include "DataModel.h"

class InfoHandleThread : public QThread
{
	Q_OBJECT

public:

	explicit InfoHandleThread(QObject *parent=0);
	~InfoHandleThread();

	void initDataAndTable(QStringList &tableNameList,QStringList &sqlList,DataModel &dm);

	void handleOnlineStatus(QStringList &tableNameList,QStringList &sqlList,DataModel &dm);

	void handleChannelState(QStringList &tableNameList,QStringList &sqlList,DataModel &dm);

	void handleDPCAction(QStringList &tableNameList,QStringList &sqlList,DataModel &dm);

	void handleFeederValue(QStringList &tableNameList,QStringList &sqlList,DataModel &dm);
protected:
	virtual void run();
};

#endif