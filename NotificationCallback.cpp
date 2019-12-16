#include "NotificationCallback.h"
#include "DataModel.h"
#include "InfoEnum.h"
#include "ToolUtil.h"

extern  QList<DataModel> infoList;
extern  QMutex infoCacheMutex;
extern QMutex initMutex;
extern Database *database;
extern Logger hisRecordLog;

extern QMap<ObId, Terminal> terminals;
extern QMap<ObId, DPCPoint> dpcPoints;
extern QMap<ObId, Feeder> feeders;
extern QMap<ObId, MVPointOfFeeder> mvPointOfFeeders;

void notificationOnlineStatus(const Notification* notif, void* clientdata)
{
	QMutexLocker lockerInit(&initMutex);//��ʼ�����������ȶ��Կ���
	ObId obId = notif->getDataItems()->getObId();
	if (terminals.contains(obId))
	{
		try
		{
			IntegerData devicePropertyData = -1;
			ToolUtil::databaseRead(obId, AT_DeviceProperty, &devicePropertyData);

			if ((int)devicePropertyData==2)//����ǹ���ָʾ��
			{
				IntegerData data = *(IntegerData*)(notif->getData());
				IntegerData oldData = *(IntegerData*)(notif->getOldData());
				QDateTime time = ToolUtil::convertTimespecToQDatetime(notif->getSequenceTime());

				DataModel dm;
				dm.enumKey = DMSCommunicateUnit_OnlineStatus;
				dm.obId = obId;
				dm.data = QString::number(data);
				dm.oldData = QString::number(oldData);
				dm.time = time;

				if (infoList.size()<(ToolUtil::getHisRecordConf("/InfoQueue/size")).toInt())
				{
					QMutexLocker locker(&infoCacheMutex);
					infoList.append(dm);
					qDebug()<<infoList.size();
				}	
			}
		}
		catch (Exception& e)
		{
			LOG4CPLUS_ERROR(hisRecordLog, "error: read AT_DeviceProperty error!!!");
		}
	}
}

void notificationChannelState(const Notification* notif, void* clientdata)
{
	QMutexLocker lockerInit(&initMutex);//��ʼ�����������ȶ��Կ���
	ObId obId = notif->getDataItems()->getObId();
	if (terminals.contains(obId))
	{
		try
		{
			IntegerData devicePropertyData = -1;
			ToolUtil::databaseRead(obId, AT_DeviceProperty, &devicePropertyData);

			if ((int)devicePropertyData!=2)//�����FTU DTU��
			{
				IntegerData data = *(IntegerData*)(notif->getData());
				IntegerData oldData = *(IntegerData*)(notif->getOldData());
				QDateTime time = ToolUtil::convertTimespecToQDatetime(notif->getSequenceTime());

				DataModel dm;
				dm.enumKey = DMSCommunicateUnit_ChannelState;
				dm.obId = obId;
				dm.data = QString::number(data);
				dm.oldData = QString::number(oldData);
				dm.time = time;

				if (infoList.size()<(ToolUtil::getHisRecordConf("/InfoQueue/size")).toInt())
				{
					QMutexLocker locker(&infoCacheMutex);
					infoList.append(dm);
					qDebug()<<infoList.size();
				}			
			}
		}
		catch (Exception& e)
		{
			LOG4CPLUS_ERROR(hisRecordLog, "error: read AT_DeviceProperty error!!!");
		}
	}
	


	//InfoEnum en = DMSCommunicateUnit_OnlineStatus;
	//switch(en)
	//{
	//case DMSCommunicateUnit_OnlineStatus:qDebug()<<"no1";break;
	//case DMSCommunicateUnit_ChannelState:qDebug()<<"no2";break;
	//}

	
	
	//Request req;
	//IntegerData iData;
	//timespec_t time;
	//req.set(dpcPointObId,AT_State,&iData,NULL,0,NORMAL_WRITE,&time);
	//database->read(&req);
	//TimeData td;
	//database->read((ObId)1713691951398,AT_OnlineTime,&td);
	//std::string aaa = (std::string)(td);
	//QString a = QString::fromUtf8(aaa.c_str());
	//qDebug()<<a;

	//StringData sd;
	//database->read((ObId)1713691951398,AT_Name,&sd);
	//std::string aa = (std::string)sd;
	//QString b = QString::fromUtf8(aa.c_str());
	//qDebug()<<b;

	//QDateTime dt = QDateTime::fromTime_t (time.tv_sec);
	//qDebug()<<iData<<dt;
	////infoList.append("1");
	////return;
	//qDebug()<<infoList.size();
}

void notificationDPCAction(const Notification* notif, void* clientdata)
{
	QMutexLocker lockerInit(&initMutex);//��ʼ�����������ȶ��Կ���
	ObId obId = notif->getDataItems()->getObId();
	if (dpcPoints.contains(obId))
	{
		try
		{
			IntegerData data = *(IntegerData*)(notif->getData());
			IntegerData oldData = *(IntegerData*)(notif->getOldData());
			QDateTime time = ToolUtil::convertTimespecToQDatetime(notif->getSequenceTime());

			DataModel dm;
			dm.enumKey = DPCPoint_ControlResult;
			dm.obId = obId;
			dm.data = QString::number(data);
			dm.oldData = QString::number(oldData);
			dm.time = time;

			if (infoList.size()<(ToolUtil::getHisRecordConf("/InfoQueue/size")).toInt())
			{
				QMutexLocker locker(&infoCacheMutex);
				infoList.append(dm);
				qDebug()<<infoList.size();
			}			
		}
		catch (Exception& e)
		{
			LOG4CPLUS_ERROR(hisRecordLog, "error: read AT_ControlResult error!!!");
		}
	}
}

void notificationMVPointValue(const Notification* notif, void* clientdata)
{
	QMutexLocker lockerInit(&initMutex);//��ʼ�����������ȶ��Կ���
	ObId obId = notif->getDataItems()->getObId();
	if (mvPointOfFeeders.contains(obId))
	{
		ObId feederObId = mvPointOfFeeders[obId].feeder.toLongLong();
		if (feeders.contains(feederObId))
		{
			try
			{
				FloatData data = *(FloatData*)(notif->getData());
				FloatData oldData = *(FloatData*)(notif->getOldData());
				QDateTime time = ToolUtil::convertTimespecToQDatetime(notif->getSequenceTime());
				
				QString loadStatOld = feeders[feederObId].loadStat;
				QString loadStatNew = "";
				if (data!=0.0)
				{
					if (feeders[feederObId].ampRating.toFloat()!=0)
					{
						float loadRate  = qAbs((float)data)/qAbs(feeders[feederObId].ampRating.toFloat());
						if (loadRate>=0.75)
						{
							loadStatNew = "1";//���أ����������75%
						}else{
							loadStatNew = "0";
						}
					}else{
						loadStatNew = "-2";//�����Ϊ�㣬�쳣
					}
				}else{
					loadStatNew = "-1";//����Ϊ�㣬��·״̬Ϊ�޵���״̬������·�Ͽ����쳣
				}

				if (loadStatNew!=loadStatOld)//״̬�����ı�
				{
					if (infoList.size()<(ToolUtil::getHisRecordConf("/InfoQueue/size")).toInt())
					{
						DataModel dm;
						dm.enumKey = MVPoint_Value;
						dm.obId = feederObId;
						dm.data = loadStatNew;
						//dm.oldData = QString::number(oldData);
						dm.time = time;
						QMutexLocker locker(&infoCacheMutex);
						infoList.append(dm);
						qDebug()<<infoList.size();
					}
				}			
			}
			catch (Exception& e)
			{
				LOG4CPLUS_ERROR(hisRecordLog, "error: read AT_mvpoint_value error!!!");
			}
		}		
	}
}

//��������ʱ���ص����³�ʼ��
void notificationIncrementCommit(const Notification* notif, void* clientdata)
{
	//��ʼ�����ݽṹ
	//�����ʼ����Ϣ����Ϣ����,��֪�߳�A����״̬��ĳ�ʼ������	
	//if (infoList.size()<(ToolUtil::getHisRecordConf("/InfoQueue/size")).toInt())
	//{
		DataModel dm;
		dm.enumKey=Init_Table;
		QMutexLocker locker(&infoCacheMutex);
		infoList.append(dm);
		ToolUtil::myDebug("start increment commit");
		//ToolUtil::myDebug(infoList.size());
	//}	
	
}
