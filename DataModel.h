#pragma once

#include "common.h"
#include "InfoEnum.h"

class DataModel
{
public:
	InfoEnum enumKey;
	ObId obId;
	QString data;
	QString oldData;
	QDateTime time;

	DataModel()
	{
		this->enumKey = DEFAULT;
		this->obId = 0;
		this->data = "''";
		this->oldData = "''";
		this->time = QDateTime::currentDateTime();
	}
};

class Terminal
{
public:
	QString serviceId;
	QString obId;
	QString onlineStat;
	QString onlineTime;
	QString workStat;
	QString workTime;
	QString type;
	QString model;
	QString factory;
	QString feeder;
	QString station;
	QString area;

	Terminal()
	{
		this->serviceId = "0";
		this->obId = "0";
		this->onlineStat = "''";
		this->onlineTime = "";
		this->workStat = "''";
		this->workTime = "";
		this->type = "''";
		this->model = "''";
		this->factory = "''";
		this->feeder = "''";
		this->station = "''";
		this->area = "''";
	}
};

class DPCPoint
{
public:
	QString serviceId;
	QString obId;
	QString station;
	QString area;

	DPCPoint()
	{
		this->serviceId = "0";
		this->obId = "0";
		this->station = "''";
		this->area = "''";
	}
};

class Feeder
{
public:
	QString serviceId;
	QString obId;
	QString ampRating;//额定电流
	QString loadStat;//负荷状态
	QString loadTime;
	QString mvPoint;//关联出口开关的A相电流遥测点
	QString station;
	QString area;

	Feeder()
	{
		this->serviceId = "0";
		this->obId = "0";
		this->ampRating = "''";
		this->loadStat = "''";
		this->loadTime = "";
		this->mvPoint = "''";
		this->station = "''";
		this->area = "''";
	}
};

class MVPointOfFeeder
{
public:
	QString serviceId;
	QString obId;
	QString feeder;

	MVPointOfFeeder()
	{
		this->serviceId = "0";
		this->obId = "0";
		this->feeder = "''";
	}
};
