/*我是UTF8无签名编码*/
#include "GEGUID64.h"
#include "GEProcess.h"
#include "GEDateTime.h"

#define RUN_BEGIN_TIME		1298908800UL	//2012年3月1日
#define RUN_SECOND			278899200UL		//可分配的秒数（到2020年1月1日）
#define RUN_ID_PER_SECOND	1009235UL		//每秒钟可分配的ID数 = 2^48 / 可分配的秒数


GEGUID64::GEGUID64(void)
	: m_uBaseID(MAX_UINT64)
	, m_uMaxID(0)
{
}

GEGUID64::~GEGUID64(void)
{
}

void GEGUID64::AllotFromNow()
{
	// 当前分配的ID从（当前时间 - 基础时间 - 1） * 每秒可分配的ID数
	GE::Uint64 RIPS = RUN_ID_PER_SECOND;
	GE::Uint64 NS = GEDateTime::Instance()->Seconds();
	m_uBaseID = RIPS * (NS - RUN_BEGIN_TIME - 1);
	// 高16位是进程ID
	GE::B8& b8 = GE_AS_B8(m_uBaseID);
	b8.UI16_3() = GEProcess::Instance()->ProcessID();
}

//计算可分配的ID最大值
void GEGUID64::AllotPerMinute()
{
	// 当前分配的ID必须小于（当前时间 - 基础时间） * 每秒可分配的ID数
	GE::Uint64 RIPS = RUN_ID_PER_SECOND;
	GE::Uint64 NS = GEDateTime::Instance()->Seconds();
	m_uMaxID = RIPS * (NS - RUN_BEGIN_TIME);
	// 高16位是进程ID
	GE::B8& b8 = GE_AS_B8(m_uMaxID);
	b8.UI16_3() = GEProcess::Instance()->ProcessID();
}

void GEGUID64::AllotGUID( GE::Uint64& b8 )
{
	// 断言分配的ID不能超过范围
	GE_ERROR(m_uBaseID < m_uMaxID);
	b8 = m_uBaseID;
	++m_uBaseID;
}

GE::Uint64 GEGUID64::AllotGUID()
{
	// 断言分配的ID不能超过范围
	GE_ERROR(m_uBaseID < m_uMaxID);
	GE::Uint64 tmp = m_uBaseID;
	++m_uBaseID;
	return tmp;
}

