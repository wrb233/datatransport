#ifndef DBTHREAD_H
#define DBTHREAD_H

#include "common.h"

class DBThread : public QThread
{
	Q_OBJECT

public:
	explicit DBThread(QObject *parent = 0);
	~DBThread();

	bool readSQLFile(QString &filePath,QStringList &sqlList);

	bool checkTable(QString tableName);

	bool createTable(QString tableName);

	void setColoum(ORMColumnDescVector &m_descVector, std::string m_strName, std::string m_strType, bool m_bNullOK=1, bool m_bIndexed=0, bool m_bKey=0, bool m_bUnique=0);

	bool createPrimaryKey(QString tableName);

	bool executeSQL(QString sql);

protected:
	virtual void run();
};
#endif