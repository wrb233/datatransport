#include "IncrementCommit.h"
#include "ToolUtil.h"
#include "datatransform.h"
#include "NotificationCallback.h"

//声明外部全局变量
extern Database *database;
extern DataTransform *dataTransform;
extern Logger datatransformLog;

bool IncrementCommit::NotfyIncrementCommit()		//注册增量提交

{
	if( database==NULL ){
		return false;
	}

	ObId faultTolerObId = 0;
	Request req;
	ObId localMacObId = 0;
	ObId incrementObId = 0;
	try
	{
		char name[32];
		ACE_OS::hostname(name, 32);
		StringData localHostName(name);
		Condition cond(AT_Name, EQ, &localHostName);
		database->find(OT_Machine, &cond, 1, &localMacObId, 1);
		if (localMacObId > 0)  
		{
			LinkData parentLink(localMacObId);
			cond.set(AT_ParentLink, EQ, &parentLink);
			database->find(OT_IncrementCommit, &cond, 1, &incrementObId, 1);
		}

		if (incrementObId > 0) {
			req.set(incrementObId, AT_CommitSyncEnd, NULL, NEW_NOTIFICATION, notificationIncrementCommit, (void*)0, NOTIFY_ON_WRITE, faultTolerObId);
			database->notify(&req, 1);
		}

	}
	catch (Exception &e) {
		localMacObId = 0;
		LOG4CPLUS_ERROR(datatransformLog, "notifyIncrementCommit() error,please check the Environment of database. " <<e.getDescription());
		return false;
		//exit(0);
	}
	return true;
}
