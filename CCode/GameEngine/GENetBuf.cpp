/*我是UTF8无签名编码*/
#include <string.h>
#include "GENetBuf.h"
#include "GEIO.h"

GENetBuf::GENetBuf( GE::Uint16 uMaxSize )
	: m_uReadSize(0)
	, m_uWriteSize(0)
	, m_uMaxSize(uMaxSize)
{
	this->m_pHead = new char[m_uMaxSize];
}

GENetBuf::~GENetBuf(void)
{
	GE_SAFE_DELETE(m_pHead);
}

void* GENetBuf::WriteFence_us()
{
	return (m_pHead + m_uWriteSize);
}

void GENetBuf::WriteBytes_us( const void* pHead, GE::Uint16 uSize )
{
	memcpy((m_pHead + m_uWriteSize), pHead, uSize);
}

void GENetBuf::MoveWriteFence_us( GE::Uint16 uSize )
{
	m_uWriteSize += uSize;
}

void* GENetBuf::ReadFence_us()
{
	return (m_pHead + m_uReadSize);
}

void GENetBuf::MoveReadFence_us( GE::Uint16 uSize )
{
	m_uReadSize += uSize;
}

void GENetBuf::InsertBytes_us( const void* pHead, GE::Uint16 uSize )
{
	memcpy(m_pHead, pHead, uSize);
}

