/*我是UTF8无签名编码*/
#include "GEGUID32.h"
#include "GEIO.h"

GEGUID32::GEGUID32( void )
	: m_uAllotID(0)
	, m_uMaxID(0)
{
}

GEGUID32::GEGUID32( GE::Uint8 uBase )
	: m_uAllotID(0)
	, m_uMaxID(0)
{
	this->SetBaseID(uBase);
}

GEGUID32::~GEGUID32( void )
{

}

void GEGUID32::SetBaseID( GE::Uint8 uBase )
{
	if (uBase == MAX_UINT8)
	{
		GE_EXC<<"GEGUID32::SetBaseID("<<uBase<<")."<<GE_END;
		return;
	}
	GE::B4& b4 = GE_AS_B4(m_uAllotID);
	b4.UI8_3() = uBase;
	GE::B4& _b4 = GE_AS_B4(m_uMaxID);
	_b4.UI8_3() = uBase + 1;
}

bool GEGUID32::AllotGUID( GE::Uint32& uID )
{
	++m_uAllotID;
	if (this->m_uAllotID < this->m_uMaxID)
	{
		uID = m_uAllotID;
		return true;
	}
	else
	{
		return false;
	}
}

