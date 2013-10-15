/*我是UTF8无签名编码*/
#include "GENetConnectMgr.h"

GENetConnectMgr::GENetConnectMgr( GE::Uint32 uMaxConnect )
	: m_IndexMgr(uMaxConnect)
{
	// 初始化容器
	this->m_pDataArr = new ConnectPtr[uMaxConnect];
	GE_ITER_UI32(_idx, uMaxConnect)
	{
		this->m_pDataArr[_idx] = NULL;
		this->m_pMutexVector.push_back(new boost::mutex);
	}
	this->m_pHasData = new bool[uMaxConnect];
	GE_ITER_UI32(uIdx, uMaxConnect)
	{
		this->m_pHasData[uIdx] = false;
	}
}

GENetConnectMgr::~GENetConnectMgr(void)
{
	
	GE_ITER_UI32(_idx, m_pMutexVector.size())
	{
		delete this->m_pMutexVector[_idx];
		this->m_pDataArr[_idx] = NULL;
		this->m_pMutexVector[_idx] = NULL;
	}
	GE_SAFE_DELETE(m_pDataArr);
}

bool GENetConnectMgr::AddConnect( GENetConnect::ConnectSharedPtr& spConnect, GE::Uint32& uID )
{
	GE::Uint32 uIdx;
	if (this->m_IndexMgr.Insert(uID, uIdx))
	{
		GE_ERROR(NULL == m_pDataArr[uIdx]);
		spConnect->SessionID(uID);
		m_pDataArr[uIdx] = spConnect.get();
		m_pDataMap.insert(std::make_pair(uID, spConnect));
		return true;
	}
	else
	{
		return false;
	}
}

bool GENetConnectMgr::DelConnect( GE::Uint32 uID )
{
	/*
	删除一个连接，加锁让线程安全
	*/
	GE::Uint32 uIdx = this->m_IndexMgr.IdxByID(uID);
	this->m_pMutexVector[uIdx]->lock();
	if (this->m_IndexMgr.Remove(uID, uIdx))
	{
		GE_ERROR(NULL != m_pDataArr[uIdx]);
		m_pDataArr[uIdx]->Shutdown(enNetConnect_LocalClose);
		m_pDataArr[uIdx] = NULL;
		m_pDataMap.erase(uID);
		this->m_pMutexVector[uIdx]->unlock();
		return true;
	}
	else
	{
		this->m_pMutexVector[uIdx]->unlock();
		return false;
	}
}

bool GENetConnectMgr::HasConnect(GE::Uint32 uID)
{
	GE::Uint32 uIdx;
	return this->m_IndexMgr.HasID(uID, uIdx);
}

GENetConnect* GENetConnectMgr::FindConnect( GE::Uint32 uID )
{
	GE::Uint32 uIdx;
	if (this->m_IndexMgr.HasID(uID, uIdx))
	{
		GE_ERROR(NULL != this->m_pDataArr[uIdx]);
		return this->m_pDataArr[uIdx];
	} 
	else
	{
		return NULL;
	}
}

GENetConnect* GENetConnectMgr::FindConnectForHasData( GE::Uint32 uID )
{
	GE::Uint32 uIdx;
	if (this->m_IndexMgr.HasID(uID, uIdx))
	{
		GE_ERROR(NULL != this->m_pDataArr[uIdx]);
		this->m_pHasData[uIdx] = true;
		return this->m_pDataArr[uIdx];
	} 
	else
	{
		return NULL;
	}
}

GENetConnect* GENetConnectMgr::NextConnect()
{
	GE::Uint32 uSessionID;
	GE::Uint32 uIdx;
	if (this->m_IndexMgr.IterNext(uSessionID, uIdx))
	{
		GE_ERROR(NULL != this->m_pDataArr[uIdx]);
		return this->m_pDataArr[uIdx];
	}
	return NULL;
}

void GENetConnectMgr::Lock( GE::Uint32 uID )
{
	GE::Uint32 uIdx = this->m_IndexMgr.IdxByID(uID);
	this->m_pMutexVector[uIdx]->lock();
}

bool GENetConnectMgr::LockForClearData( GE::Uint32 uID )
{
	GE::Uint32 uIdx = this->m_IndexMgr.IdxByID(uID);
	if (this->m_pHasData[uIdx])
	{
		this->m_pHasData[uIdx] = false;
		this->m_pMutexVector[uIdx]->lock();
		return true;
	}
	else
	{
		return false;
	}
}

void GENetConnectMgr::Unlock( GE::Uint32 uID )
{
	GE::Uint32 uIdx = this->m_IndexMgr.IdxByID(uID);
	this->m_pMutexVector[uIdx]->unlock();
}

