#include "GEIndex.h"

GEIndex::GEIndex( GE::Uint32 uMaxSize )
	: m_uSize(0)
	, m_uMaxSize(uMaxSize)
	, m_uFreeListIdx(0)
	, m_uUserListIdx(MAX_UINT32)
{
	// 最大大小不能为0
	GE_ERROR(uMaxSize);
	// 构建空闲链表
	this->m_pData = new _GEIIElem[uMaxSize];
	GE_ITER_UI32(_idx, uMaxSize)
	{
		this->m_pData[_idx].m_uPrevIdx = _idx - 1;
		this->m_pData[_idx].m_uNextIdx = _idx + 1;
		this->m_pData[_idx].m_uAllotID = _idx + 1;
	}
	this->m_pData[0].m_uPrevIdx = uMaxSize - 1;
	this->m_pData[uMaxSize - 1].m_uNextIdx = 0;
	// 占据0
	//GE::Uint32 uID = 0;
	//GE::Uint32 uIdx = 0;
	//this->Insert(uID, uIdx);
	//GE_ERROR(uID == 0);
	//GE_ERROR(uIdx == 0);
}


GEIndex::~GEIndex(void)
{
	delete [] m_pData;
}

bool GEIndex::Insert( GE::Uint32& uID, GE::Uint32& uIdx )
{
	// 如果没有空闲的节点来，则直接返回之
	if (this->m_uSize == this->m_uMaxSize)
	{
		return false;
	}
	GE::Uint32 pidx, nidx;
	// 保存空闲结点的前驱和后继索引
	pidx = this->m_pData[m_uFreeListIdx].m_uPrevIdx;
	nidx = this->m_pData[m_uFreeListIdx].m_uNextIdx;
	// 设置要分配的ID，并分配出去
	--(m_pData[m_uFreeListIdx].m_uAllotID);
	uIdx = this->m_uFreeListIdx;
	uID = this->m_pData[uIdx].m_uAllotID;
	// 将空闲结点移除空闲列表
	this->m_pData[pidx].m_uNextIdx = nidx;
	this->m_pData[nidx].m_uPrevIdx = pidx;
	this->m_uFreeListIdx = nidx;
	// 将空闲节点加入到使用列表中
	if (0 == this->m_uSize)
	{
		this->m_pData[uIdx].m_uNextIdx = uIdx;
		this->m_pData[uIdx].m_uPrevIdx = uIdx;
		this->m_uUserListIdx = uIdx;
	}
	else
	{
		pidx = this->m_pData[m_uUserListIdx].m_uPrevIdx;
		this->m_pData[uIdx].m_uPrevIdx = pidx;
		this->m_pData[uIdx].m_uNextIdx = this->m_uUserListIdx;
		this->m_pData[m_uUserListIdx].m_uPrevIdx = uIdx;
		this->m_pData[pidx].m_uNextIdx = uIdx;
	}
	// 增加一个计数
	++m_uSize;
	return true;
}

bool GEIndex::Remove( GE::Uint32 uID, GE::Uint32& uIdx )
{
	// 根据ID，获取索引位置
	uIdx = this->IdxByID(uID);
	// 如果当前的索引位置上分配出去的ID不是传进来的ID，则删除失败
	if (uID != this->m_pData[uIdx].m_uAllotID)
	{
		return false;
	}
	// 从使用列表中删除节点
	GE_ERROR(m_uSize > 0);
	GE_ERROR(m_uUserListIdx != MAX_UINT32);
	GE::Uint32 pidx, nidx;
	// 保存删除结点的前驱和后继索引
	pidx = this->m_pData[uIdx].m_uPrevIdx;
	nidx = this->m_pData[uIdx].m_uNextIdx;
	// 将要删除的节点移出使用列表
	this->m_pData[pidx].m_uNextIdx = nidx;
	this->m_pData[nidx].m_uPrevIdx = pidx;
	// 如果删除的节点是当前索引节点，则将当前节点索引移到下一个节点处
	if (m_uUserListIdx == uIdx)
	{
		m_uUserListIdx = nidx;
	}
	// 分配下次要分配的ID + 1
	this->m_pData[uIdx].m_uAllotID += (m_uMaxSize + 1);
	// 只有该位置的ID还可以继续分配的时候，才将该位置加入到空闲链表中。
	if (uID < MAX_UINT32 - 2 * m_uMaxSize - 2)
	{
		// 如果此时空闲列表是空的，则重建空闲链表
		if (this->m_uSize == this->m_uMaxSize)
		{
			this->m_pData[uIdx].m_uPrevIdx = uIdx;
			this->m_pData[uIdx].m_uNextIdx = uIdx;
			this->m_uFreeListIdx = uIdx;
		}
		// 否则，将删除的节点加入空闲列表
		else
		{
			pidx = this->m_pData[m_uFreeListIdx].m_uPrevIdx;
			this->m_pData[uIdx].m_uPrevIdx = pidx;
			this->m_pData[uIdx].m_uNextIdx = this->m_uFreeListIdx;
			this->m_pData[m_uFreeListIdx].m_uPrevIdx = uIdx;
			this->m_pData[pidx].m_uNextIdx = uIdx;
		}
	}
	--m_uSize;
	return true;
}

bool GEIndex::HasID( GE::Uint32 uID, GE::Uint32& uIdx )
{
	uIdx = this->IdxByID(uID);
	return this->m_pData[uIdx].m_uAllotID == uID;
}

bool GEIndex::HasIndex( GE::Uint32& uID, GE::Uint32 uIdx )
{
	uID = this->m_pData[uIdx].m_uAllotID;
	return uID % m_uMaxSize == uIdx;
}

bool GEIndex::IterNext( GE::Uint32& uID, GE::Uint32& uIdx )
{
	if (0 == m_uSize)
	{
		return false;
	}
	uIdx = this->m_uUserListIdx;
	uID = this->m_pData[uIdx].m_uAllotID;
	this->m_uUserListIdx = this->m_pData[uIdx].m_uNextIdx;
	return true;
}

bool GEIndex::IterPrev( GE::Uint32& uID, GE::Uint32& uIdx )
{
	if (0 == m_uSize)
	{
		return false;
	}
	uIdx = this->m_uUserListIdx;
	uID = this->m_pData[uIdx].m_uAllotID;
	this->m_uUserListIdx = this->m_pData[uIdx].m_uPrevIdx;
	return true;
}

