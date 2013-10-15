/************************************************************************
时间模块
************************************************************************/
#pragma once
#include "GESingleton.h"
#include "GESingleton.h"
#include "GEPython.h"

class GEDateTime
	: public GESingleton<GEDateTime>
{
public:
	GEDateTime(void);
	~GEDateTime(void);

public:
	static void			SleepMsec(GE::Uint32 uMsec);										//休眠
	void				Refresh();															//刷新时间，并缓存之
	// 获取当前时间的函数簇
	GE::Uint32			Year() {return m_uYear;}											//年（年份，如2011）
	GE::Uint32			Month() {return m_uMonth;}											//月（月份，1 -- 12）
	GE::Uint32			Day() {return m_uDay;}												//日（日期，1 -- 31）
	GE::Uint32			Hour() {return m_uHour;}											//时（小时，0 -- 23）
	GE::Uint32			Minute() {return m_uMinute;}										//分（分钟，0 -- 59）
	GE::Uint32			Second() {return m_uSecond;}										//秒（秒钟，0 -- 59）
	GE::Uint32			WeekDay() {return m_uWeekDay;}										//星期几（0，星期天；1，星期1 ...）
	GE::Uint32			YeayDay() {return m_uYearDay;}										//今年的第几天
	GE::Uint32			Seconds() {return m_uUnixTime;}										//从1970元到现在的秒数
	GE::Uint32			Minutes() {return m_uUnixTime / 60;}								//从1970元到现在的分钟数
	GE::Uint32			Days() {return (m_uUnixTime + m_uTimeZoneSecond) / 86400;}			//从1970元到现在的天数(注意这里修正了时区的影响)
	GE::Uint64			MSeconds() {return m_uCPUCLock;}									//进程启动到现在的毫秒数
	GE::Uint32			TimeZoneSeconds() {return m_uTimeZoneSecond;}						//服务端当前进程所在计算机的时区
	GEPython::Object&	Now() {return m_PyNow;}												//现在的时间（Python）
	// 修正时间的函数
	void				SetUnixTime(GE::Uint32 uUTCTime = 0);								//设置内部时间

private:
	void				CasheClock();
	void				CasheTime();

private:
	GE::Uint32			m_uYear;
	GE::Uint32			m_uMonth;
	GE::Uint32			m_uDay;
	GE::Uint32			m_uHour;
	GE::Uint32			m_uMinute;
	GE::Uint32			m_uSecond;
	GE::Uint32			m_uWeekDay;
	GE::Uint32			m_uYearDay;

	GE::Uint32			m_uUnixTime;		//当前的UTC时间（非系统时间，内部计算的）
	GE::Uint64			m_uCPUCLock;		//根据CPU震荡周期保存的进程/系统启动的毫秒数

	GE::Uint64			m_uCumulation;		//用于累加毫秒的累加器
	bool				m_bIsCumulatino;	//标志是否需要累加
	GE::Uint32			m_uTimeZoneSecond;	//修正时区对计算天数的影响

	GEPython::Object	m_PyNow;			//缓存Python的datetime对象
};

// 毫秒级别计时(不会受GEDataTime的时间暂停的影响)
class GEKeepMsec
{
	GE_DISABLE_BOJ_CPY(GEKeepMsec);
public:
	GEKeepMsec(void) {Reset();}
	~GEKeepMsec(void) {}

public:
	void			Reset() {m_uStartMsec = GEDateTime::Instance()->MSeconds();}			//重置记时器
	GE::Uint64		PassMsec() {return GEDateTime::Instance()->MSeconds() - m_uStartMsec;}	//上次重置计时器到现在的毫秒数
	bool			HasPass(GE::Uint64 msec)												//是否已经过了msec毫秒
	{
		if(m_uStartMsec + msec > GEDateTime::Instance()->MSeconds())
		{
			return false;
		}
		m_uStartMsec += msec;
		return true;
	}

private:
	GE::Uint64			m_uStartMsec;
};

// 秒级别计时（会受GEDataTime的时间暂停的影响）
class GEKeepSec
{
	GE_DISABLE_BOJ_CPY(GEKeepSec);
public:
	GEKeepSec(void) {Reset();}
	~GEKeepSec(void) {}

public:
	void			Reset() {m_uStartSec = GEDateTime::Instance()->Second();}				//重置记时器
	GE::Uint64		PassSec() {return GEDateTime::Instance()->Second() - m_uStartSec;}		//上次重置计时器到现在的秒数
	bool			HasPass(GE::Uint64 sec)													//是否已经过了sec秒
	{
		if(m_uStartSec + sec > GEDateTime::Instance()->Second())
		{
			return false;
		}
		m_uStartSec += sec;
		return true;
	}

private:
	GE::Uint64			m_uStartSec;
};

