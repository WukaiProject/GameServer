/*我是UTF8无签名编码*/
#include <time.h>
#include <boost/thread/thread.hpp>
#include <boost/timer/timer.hpp>

#include "GEDateTime.h"

//////////////////////////////////////////////////////////////////////////
// GEDateTime
//////////////////////////////////////////////////////////////////////////
GEDateTime::GEDateTime(void)
	: m_uCumulation(0)
	, m_bIsCumulatino(false)
{
	// 消除时区的影响
	this->m_uUnixTime = 0;
	this->CasheTime();
	this->m_uTimeZoneSecond = this->m_uHour * 3600 + this->m_uMinute * 60 + this->m_uSecond;
	// 缓存当前时间
	this->m_uUnixTime = static_cast<GE::Uint32>(time(0));
	this->CasheClock();
	this->CasheTime();
}

GEDateTime::~GEDateTime(void)
{
}

void GEDateTime::SleepMsec( GE::Uint32 uMsec )
{
	boost::this_thread::sleep(boost::posix_time::milliseconds(uMsec));
}

void GEDateTime::Refresh()
{

	GE::Uint64 uTmpClock = this->m_uCPUCLock;
	this->CasheClock();
#if WIN
	if (m_uCPUCLock < uTmpClock)
	{
		std::cout<<"Time Cycle."<<std::endl;
		return;
	}
#endif
	if (m_bIsCumulatino)
	{
		// 注意，这里是无符号整型的相减，即便是m_uCPUCLock < uTmpClock, 得到的结果还是所需的时间差
		m_uCumulation += (m_uCPUCLock - uTmpClock);
		/*
		如果过了1秒以上,则计算时间
		这样1秒1秒的的计算，可以让服务器能够追帧
		*/
		if ( m_uCumulation > 1000)
		{
			m_uUnixTime += 1;
			m_uCumulation -= 1000;
			this->CasheTime();
		}
	}
}

void GEDateTime::SetUnixTime( GE::Uint32 uUTCTime /*= 0*/ )
{
	if (uUTCTime)
	{
		// 时间必须单调递增
		GE_ERROR(uUTCTime >= this->m_uUnixTime);
		this->m_uUnixTime = uUTCTime;
		this->Refresh();
		this->CasheTime();
		this->m_bIsCumulatino = true;
	}
	else
	{
		this->m_bIsCumulatino = false;
	}
}

void GEDateTime::CasheClock()
{
#ifdef WIN
	this->m_uCPUCLock = static_cast<GE::Uint64>(clock());
#elif LINUX
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	this->m_uCPUCLock = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
#endif
}

void GEDateTime::CasheTime()
{
	// 缓存C++时间
	time_t _tt = static_cast<time_t>(m_uUnixTime);
	tm* _tm = localtime(&_tt);
	this->m_uYear = _tm->tm_year + 1900;
	this->m_uMonth = _tm->tm_mon + 1;
	this->m_uDay = _tm->tm_mday;
	this->m_uHour = _tm->tm_hour;
	this->m_uMinute = _tm->tm_min;
	this->m_uSecond = _tm->tm_sec;
	this->m_uWeekDay = _tm->tm_wday;
	this->m_uYearDay = _tm->tm_yday;
	/*
	Return value: New reference.
	缓存Python时间
	*/
	m_PyNow.SetObj_NewRef(GEPython::PyObjFromDatetime(this->Year(), this->Month(), this->Day(), this->Hour(), this->Minute(), this->Second()));
}

