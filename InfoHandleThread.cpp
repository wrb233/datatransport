#include "InfoHandleThread.h"
#include "ToolUtil.h"
#include "datatransform.h"

extern DataTransform* dataTransform;
extern DBPOOLHANDLE dbPoolHandle;
extern Database *database;
extern  QMutex infoCacheMutex;
extern  QList<DataModel> infoList;
extern QMutex initMutex;

extern QMap<ObId, Terminal> terminals;
extern QMap<ObId, DPCPoint> dpcPoints;
extern QMap<ObId, Feeder> feeders;
extern QMap<ObId, MVPointOfFeeder> mvPointOfFeeders;

InfoHandleThread::InfoHandleThread(QObject *parent):QThread(parent)
{
	qDebug() << "InfoHandle Thread : " << QThread::currentThreadId();
}

InfoHandleThread::~InfoHandleThread()
{

}

void InfoHandleThread::run()  {
	qDebug() << "InfoHandle Run Thread : " << QThread::currentThreadId();
	while (true)
	{
		if (infoList.size()>0)
		{
			QList<DataModel> infoListTemp;
			qDebug() << "InfoHandle Thread is Working: " << QThread::currentThreadId();
			{
				QMutexLocker locker(&infoCacheMutex);
				infoListTemp = infoList;
				infoList.clear();
			}
			QStringList tableNameList;
			QStringList sqlList;
			foreach(DataModel dm,infoListTemp)
			{
				switch(dm.enumKey)
				{
				case Init_Table:initDataAndTable(tableNameList,sqlList,dm);break;
				case DMSCommunicateUnit_OnlineStatus:handleOnlineStatus(tableNameList,sqlList,dm);break;
				case DMSCommunicateUnit_ChannelState:handleChannelState(tableNameList,sqlList,dm);break;
				case DPCPoint_ControlResult:handleDPCAction(tableNameList,sqlList,dm);break;
				case MVPoint_Value:handleFeederValue(tableNameList,sqlList,dm);break;
				}
			}
			dataTransform->writeSQLFileByInfo(tableNameList,sqlList);
		}
		msleep(30000);//30s
	}
}

void InfoHandleThread::initDataAndTable(QStringList &tableNameList,QStringList &sqlList,DataModel &dm)
{
	ToolUtil::myDebug("MethodName:initTable start");
	QMutexLocker locker(&initMutex);//加锁，防止在程序刚启动的瞬间，初始化未完成的极其短暂的时间内，有回调进来，保证初始化状态绝对正确
	//初始化前，必要清理
	terminals.clear();
	dpcPoints.clear();
	feeders.clear();
	mvPointOfFeeders.clear();

	//初始化table
	QStringList tableNames;
	tableNames<< "H_TERMINAL_RUNTIME"<<"H_FEEDER_RUNTIME";
	foreach(QString tableName, tableNames)
	{
		if (!tableNameList.contains(tableName))
		{
			tableNameList.append(tableName);
		}
	}

	sqlList.append("DELETE FROM H_TERMINAL_RUNTIME");//清空表数据
	sqlList.append("DELETE FROM H_FEEDER_RUNTIME");

	//初始化终端
	int numOfTerminal = ToolUtil::databaseFind(OT_DMSCommunicateUnit, NULL, 0);
	if (numOfTerminal>0)
	{
		ObId *obIdOfTerminal = new ObId[numOfTerminal];
		ToolUtil::databaseFind(OT_DMSCommunicateUnit, NULL, 0, obIdOfTerminal, numOfTerminal);
		for (int i=0;i<numOfTerminal;i++)
		{
			Terminal tm;
			tm.serviceId = QString::number(obIdOfTerminal[i]);
			tm.obId = QString::number(obIdOfTerminal[i]);
			IntegerData deviceProperty = -1;
			ToolUtil::databaseRead(obIdOfTerminal[i], AT_DeviceProperty, &deviceProperty);
			tm.type = QString::number((int)deviceProperty);
			if ((int)deviceProperty==2)//如果是故障指示器
			{
				IntegerData onlineState = -1;
				timespec_t time;
				ToolUtil::databaseReadTime(obIdOfTerminal[i], AT_OnlineStatus, &onlineState, time);
				tm.onlineStat = QString::number((int)onlineState);
				tm.onlineTime = ToolUtil::convertTimespecToQDatetime(time).toString("yyyy-MM-dd hh:mm:ss.zzz");
			}else{
				IntegerData channelState = -1;
				timespec_t time;
				ToolUtil::databaseReadTime(obIdOfTerminal[i], AT_ChannelState, &channelState, time);
				tm.onlineStat = QString::number((int)channelState);
				tm.onlineTime = ToolUtil::convertTimespecToQDatetime(time).toString("yyyy-MM-dd hh:mm:ss.zzz");
			}
			TimeData timeData;
			if (ToolUtil::databaseRead(obIdOfTerminal[i], AT_OnlineTime, &timeData))
			{
				tm.workTime = QDateTime::fromTime_t(timeData.operator timespec_t().tv_sec).toString("yyyy-MM-dd hh:mm:ss");
				qDebug()<<tm.workTime;
				if (tm.workTime=="1970-01-01 08:00:00")
				{
					tm.workStat = "0";
				}else{
					tm.workStat = "1";
				}	
			}else{
				tm.workStat = "-1";//读取异常
			}
		
			StringData model = "";
			ToolUtil::databaseRead(obIdOfTerminal[i], AT_Model, &model);
			tm.model = QString::fromUtf8(((std::string)model).c_str());
			StringData factory = "";
			ToolUtil::databaseRead(obIdOfTerminal[i], AT_Factory, &factory);
			tm.factory = QString::fromUtf8(((std::string)factory).c_str());
			LinkData feederLink = 0;
			ToolUtil::databaseRead(obIdOfTerminal[i], AT_FeederLink, &feederLink);
			tm.feeder = QString::number((ObId)feederLink);
			ObId subStationObId = ToolUtil::findOTypeByObId((ObId)feederLink,OT_PMSStation);
			tm.station = QString::number(subStationObId);
			ObId areaObId = ToolUtil::findOTypeByObId((ObId)feederLink,OT_SubControlArea);
			tm.area =  QString::number(areaObId);

			terminals.insert(obIdOfTerminal[i], tm);

			QString sql = "INSERT INTO H_TERMINAL_RUNTIME\
						  (SERVICEID, OBID,ONLINESTAT,ONLINETIME,WORKSTAT,WORKTIME,TYPE,MODEL,FACTORY,FEEDER,STATION,AREA) \
						  VALUES (%1,%2,%3,%4,%5,%6,%7,'%8','%9',%10,%11,%12) ";
			sql = sql.arg(tm.serviceId).arg(tm.obId).arg(tm.onlineStat).arg("to_timestamp('"+tm.onlineTime+"', 'yyyy-mm-dd hh24:mi:ss.ff')")
				.arg(tm.workStat).arg("to_timestamp('"+tm.workTime+"', 'yyyy-mm-dd hh24:mi:ss.ff')")
				.arg(tm.type).arg(tm.model).arg(tm.factory).arg(tm.feeder).arg(tm.station).arg(tm.area);
			sqlList.append(sql);
		}

		delete[] obIdOfTerminal;	
	}	

	//初始化遥控
	int numOfDPC = ToolUtil::databaseFind(OT_DPCPoint, NULL, 0);
	if (numOfDPC>0)
	{
		ObId *obIdOfDPC = new ObId[numOfDPC];
		ToolUtil::databaseFind(OT_DPCPoint, NULL, 0, obIdOfDPC, numOfDPC);
		for (int i=0;i<numOfDPC;i++)
		{
			DPCPoint dpc;
			dpc.serviceId = QString::number(obIdOfDPC[i]);
			dpc.obId = QString::number(obIdOfDPC[i]);
			LinkData dpsPointLink = 0;
			ToolUtil::databaseRead(obIdOfDPC[i], AT_DPSPointLink, &dpsPointLink);
			if (dpsPointLink!=0)
			{
				LinkData psrLink = 0;
				ToolUtil::databaseRead((ObId)dpsPointLink, AT_DPSPointLink, &psrLink);
				if (psrLink!=0)
				{
					ObId subStationObId = ToolUtil::findOTypeByObId((ObId)psrLink,OT_PMSStation);
					dpc.station = QString::number(subStationObId);
					ObId areaObId = ToolUtil::findOTypeByObId((ObId)psrLink,OT_SubControlArea);
					dpc.area =  QString::number(areaObId);
				}
			}
			dpcPoints.insert(obIdOfDPC[i], dpc);
		}
		delete[] obIdOfDPC;	
	}
	//初始化馈线
	int numOfFeeder= ToolUtil::databaseFind(OT_Feeder, NULL, 0);
	if (numOfFeeder>0)
	{
		ObId *obIdOfFeeder = new ObId[numOfFeeder];
		ToolUtil::databaseFind(OT_Feeder, NULL, 0, obIdOfFeeder, numOfFeeder);
		for (int i=0;i<numOfFeeder;i++)
		{
			Feeder fd;
			fd.serviceId = QString::number(obIdOfFeeder[i]);
			fd.obId = QString::number(obIdOfFeeder[i]);
			ObId subStationObId = ToolUtil::findOTypeByObId(obIdOfFeeder[i],OT_PMSStation);
			fd.station = QString::number(subStationObId);
			ObId areaObId = ToolUtil::findOTypeByObId(obIdOfFeeder[i],OT_SubControlArea);
			fd.area =  QString::number(areaObId);

			ContainerData childrenListOfFeeder;
			ToolUtil::databaseRead(obIdOfFeeder[i], AT_ChildrenList, &childrenListOfFeeder);
			int sonNumOfFeeder = childrenListOfFeeder.getNumberOfElements();
			if (sonNumOfFeeder>0)
			{
				ObId pmsBreaker = 0;//出口开关obid
				const ObId *childrenObIdOfFeeder = childrenListOfFeeder.getObIds();
				for (int j=0;j<sonNumOfFeeder;j++)
				{
					OType sonOTypeOfFeeder = ToolUtil::databaseExtractOType(childrenObIdOfFeeder[j]);
					if (sonOTypeOfFeeder==OT_PMSBreaker)
					{
						IntegerData breakType = -1;
						ToolUtil::databaseRead(childrenObIdOfFeeder[j], AT_BreakerType, &breakType);
						if (6==breakType)//出口开关类型
						{
							pmsBreaker = childrenObIdOfFeeder[j];
							FloatData ampRateDate = 0.0;
							ToolUtil::databaseRead(childrenObIdOfFeeder[j], AT_ampRating, &ampRateDate);
							fd.ampRating = QString::number(ampRateDate);
							break;
						}
					}
				}
				if (pmsBreaker!=0)
				{
					ContainerData childrenListOfBreaker;
					ToolUtil::databaseRead(pmsBreaker, AT_ChildrenList, &childrenListOfBreaker);
					int sonNumOfBreaker = childrenListOfBreaker.getNumberOfElements();
					if (sonNumOfBreaker>0)
					{
						const ObId *childrenObIdOfBreaker = childrenListOfBreaker.getObIds();
						for (int k=0;k<sonNumOfBreaker;k++)
						{
							OType sonOTypeOfBreaker = ToolUtil::databaseExtractOType(childrenObIdOfBreaker[k]);
							if (sonOTypeOfBreaker==OT_PMSTerminal)
							{
								ContainerData childrenListOfTerminal;
								ToolUtil::databaseRead(childrenObIdOfBreaker[k], AT_ChildrenList, &childrenListOfTerminal);
								int sonNumOfTerminal = childrenListOfTerminal.getNumberOfElements();
								if (sonNumOfTerminal>0)
								{
									const ObId *childrenObIdOfTerminal = childrenListOfTerminal.getObIds();
									for (int m=0;m<sonNumOfTerminal;m++)
									{
										OType sonOTypeOfTerminal = ToolUtil::databaseExtractOType(childrenObIdOfTerminal[m]);
										if (sonOTypeOfTerminal==OT_Analog)
										{
											LinkData measureTypeLink = 0;
											ToolUtil::databaseRead(childrenObIdOfTerminal[m], AT_MeasurementTypeLink, &measureTypeLink);
											if (measureTypeLink!=0)
											{
												StringData keyName;
												ToolUtil::databaseRead((ObId)measureTypeLink, AT_KeyName, &keyName);
												if ((std::string)keyName=="I_A")//A相电流
												{
													LinkData measLink = 0;
													ToolUtil::databaseRead(childrenObIdOfTerminal[m], AT_MeasLink, &measLink);
													if (measLink!=0)
													{
														MVPointOfFeeder mvOFd;
														mvOFd.serviceId = QString::number((ObId)measLink);
														mvOFd.obId = QString::number((ObId)measLink);
														mvOFd.feeder = fd.obId;
														mvPointOfFeeders.insert((ObId)measLink, mvOFd);
														fd.mvPoint = QString::number((ObId)measLink);

														FloatData valueData = 0.0;
														timespec_t time;
														ToolUtil::databaseReadTime((ObId)measLink, AT_Value, &valueData, time);
														fd.loadTime = ToolUtil::convertTimespecToQDatetime(time).toString("yyyy-MM-dd hh:mm:ss.zzz");
														if (valueData!=0.0)
														{
															if (fd.ampRating!=0)
															{
																float loadRate  = qAbs((float)valueData)/qAbs(fd.ampRating.toFloat());
																if (loadRate>=0.75)
																{
																	fd.loadStat = "1";//重载，超过额定电流75%
																}else{
																	fd.loadStat = "0";
																}
															}else{
																fd.loadStat = "-2";//额定电流为零，异常
															}
														}else{
															fd.loadStat = "-1";//电流为零，线路状态为无电流状态，该线路断开或异常
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
			feeders.insert(obIdOfFeeder[i], fd);

			QString sql = "INSERT INTO H_FEEDER_RUNTIME\
						  (SERVICEID, OBID,AMPRATING,LOADSTAT,LOADTIME,MVPOINT,STATION,AREA) \
						  VALUES (%1,%2,%3,%4,%5,%6,%7,%8) ";
			sql = sql.arg(fd.serviceId).arg(fd.obId).arg(fd.ampRating).arg(fd.loadStat).arg("to_timestamp('"+fd.loadTime+"', 'yyyy-mm-dd hh24:mi:ss.ff')")
				.arg(fd.mvPoint).arg(fd.station).arg(fd.area);
			sqlList.append(sql);

		}
		delete[] obIdOfFeeder;	
	}
	//初始化馈线出口开关的A相电流遥控
	dataTransform->writeSQLFileByInfo(tableNameList,sqlList);
	ToolUtil::myDebug("MethodName:initTable end");
}

void InfoHandleThread::handleOnlineStatus(QStringList &tableNameList,QStringList &sqlList,DataModel &dm)
{
	ToolUtil::myDebug("MethodName:handleOnlineStatus start");
	QString tableTime = dm.time.toString("yyyy");
	QString tableName = "H_TERMINAL_RECORD_"+tableTime;
	if (!tableNameList.contains(tableName))
	{
		tableNameList.append(tableName);
	}

	if (terminals.contains(dm.obId))
	{
		QString updateSQL = "UPDATE H_TERMINAL_RUNTIME SET ONLINESTAT=%1,ONLINETIME=%2 WHERE OBID=%3";
		QString insertSQL = "INSERT INTO %1\
					  (SERVICEID, OBID,ONLINESTAT,ONLINETIME,TYPE,MODEL,FACTORY,FEEDER,STATION,AREA) \
					  VALUES (%2,%3,%4,%5,%6,'%7','%8',%9,%10,%11) ";
		terminals[dm.obId].onlineStat = dm.data;
		terminals[dm.obId].onlineTime = dm.time.toString("yyyy-MM-dd hh:mm:ss.zzz");

		updateSQL = updateSQL.arg(terminals[dm.obId].onlineStat).arg("to_timestamp('"+terminals[dm.obId].onlineTime+"', 'yyyy-mm-dd hh24:mi:ss.ff')").arg(terminals[dm.obId].obId);
		insertSQL = insertSQL.arg(tableName).arg(terminals[dm.obId].serviceId).arg(terminals[dm.obId].obId).arg(terminals[dm.obId].onlineStat)
			.arg("to_timestamp('"+terminals[dm.obId].onlineTime+"', 'yyyy-mm-dd hh24:mi:ss.ff')").arg(terminals[dm.obId].type).arg(terminals[dm.obId].model)
			.arg(terminals[dm.obId].factory).arg(terminals[dm.obId].feeder).arg(terminals[dm.obId].station).arg(terminals[dm.obId].area);
		sqlList.append(updateSQL);
		ToolUtil::myDebug(updateSQL);
		sqlList.append(insertSQL);
		ToolUtil::myDebug(insertSQL);
	}
	ToolUtil::myDebug("MethodName:handleOnlineStatus end");
}

void InfoHandleThread::handleChannelState(QStringList &tableNameList,QStringList &sqlList,DataModel &dm)
{
	ToolUtil::myDebug("MethodName:handleChannelState start");
	handleOnlineStatus(tableNameList,sqlList,dm);
	ToolUtil::myDebug("MethodName:handleChannelState end");
}

void InfoHandleThread::handleDPCAction(QStringList &tableNameList,QStringList &sqlList,DataModel &dm)
{
	ToolUtil::myDebug("MethodName:handleDPCAction start");
	QString tableTime = dm.time.toString("yyyy");
	QString tableName = "H_DPCPOINT_RECORD_"+tableTime;
	if (!tableNameList.contains(tableName))
	{
		tableNameList.append(tableName);
	}

	if (dpcPoints.contains(dm.obId))
	{
		QString sql = "INSERT INTO %1\
					  (SERVICEID, OBID,DPCACTION,ACTIONTIME,ACTIONRESULT,STATION,AREA) \
					  VALUES (%2,%3,%4,%5,%6,%7,%8) ";
		QString dpcAction;
		QString actionResult;
		switch(dm.data.toInt())
		{
		case 0:dpcAction= "0"; actionResult = "0"; break;//选择成功
		case 1:dpcAction= "0"; actionResult = "1"; break;//选择失败
		case 2:dpcAction= "0"; actionResult = "2"; break;//选择超时
		case 3:dpcAction= "1"; actionResult = "0"; break;//执行成功
		case 4:dpcAction= "1"; actionResult = "1"; break;//执行失败
		case 5:dpcAction= "1"; actionResult = "2"; break;//执行超时
		case 6:dpcAction= "2"; actionResult = "0"; break;//取消成功
		case 7:dpcAction= "2"; actionResult = "1"; break;//取消失败
		case 8:dpcAction= "2"; actionResult = "2"; break;//取消超时
		case 9:dpcAction= "3"; actionResult = "0"; break;//通讯中断
		case 10:dpcAction= "4"; actionResult = "0"; break;//遥控冲突
		case 11:dpcAction= "5"; actionResult = "0"; break;//任务超时
		case 12:dpcAction= "6"; actionResult = "0"; break;//调档成功
		case 13:dpcAction= "6"; actionResult = "1"; break;//调档失败
		case 14:dpcAction= "7"; actionResult = "0"; break;//急停成功
		case 15:dpcAction= "7"; actionResult = "1"; break;//急停失败
		case 16:dpcAction= "8"; actionResult = "0"; break;//选择开始
		case 17:dpcAction= "9"; actionResult = "0"; break;//执行开始
		case 18:dpcAction= "10"; actionResult = "0"; break;//取消开始
		}
		sql = sql.arg(tableName).arg(dpcPoints[dm.obId].obId).arg(dpcPoints[dm.obId].obId).arg(dpcAction)
			.arg("to_timestamp('"+dm.time.toString("yyyy-MM-dd hh:mm:ss.zzz")+"', 'yyyy-mm-dd hh24:mi:ss.ff')").arg(actionResult)
			.arg(dpcPoints[dm.obId].station).arg(dpcPoints[dm.obId].area);
		sqlList.append(sql);
	}
	ToolUtil::myDebug("MethodName:handleDPCAction end");
}

void InfoHandleThread::handleFeederValue(QStringList &tableNameList,QStringList &sqlList,DataModel &dm)
{
	ToolUtil::myDebug("MethodName:handleFeederValue start");
	QString tableTime = dm.time.toString("yyyy");
	QString tableName = "H_FEEDER_RECORD_"+tableTime;
	if (!tableNameList.contains(tableName))
	{
		tableNameList.append(tableName);
	}

	if (feeders.contains(dm.obId))
	{
		QString updateSQL = "UPDATE H_FEEDER_RUNTIME SET LOADSTAT=%1,LOADTIME=%2 WHERE OBID=%3";
		QString insertSQL = "INSERT INTO %1 (SERVICEID, OBID,LOADSTAT,LOADTIME,STATION,AREA) \
							VALUES (%2,%3,%4,%5,%6,%7) ";
		feeders[dm.obId].loadStat = dm.data;
		feeders[dm.obId].loadTime = dm.time.toString("yyyy-MM-dd hh:mm:ss.zzz");

		updateSQL = updateSQL.arg(feeders[dm.obId].loadStat).arg("to_timestamp('"+feeders[dm.obId].loadTime+"', 'yyyy-mm-dd hh24:mi:ss.ff')").arg(feeders[dm.obId].obId);
		insertSQL = insertSQL.arg(tableName).arg(feeders[dm.obId].serviceId).arg(feeders[dm.obId].obId).arg(feeders[dm.obId].loadStat)
			.arg("to_timestamp('"+feeders[dm.obId].loadTime+"', 'yyyy-mm-dd hh24:mi:ss.ff')").arg(feeders[dm.obId].station).arg(feeders[dm.obId].area);
		sqlList.append(updateSQL);
		ToolUtil::myDebug(updateSQL);
		sqlList.append(insertSQL);
		ToolUtil::myDebug(insertSQL);
	}
	ToolUtil::myDebug("MethodName:handleFeederValue end");
}