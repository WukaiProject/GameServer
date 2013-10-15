/************************************************************************
一个需要配合数组使用的下标-ID索引器
GEIndex可以将一个数组的各个下标分配为不重复的ID
GEIndex自带内部循环迭代器，迭代器不受（Insert和Remove操作影响）
************************************************************************/
#pragma once
#include "GEInteger.h"

struct _GEIIElem
{
	GE::Uint32				m_uPrevIdx;											//前驱索引
	GE::Uint32				m_uNextIdx;											//后继索引
	GE::Uint32				m_uAllotID;											//当前分配的ID
};

class GEIndex
{
	GE_DISABLE_BOJ_CPY(GEIndex);
public:
	GEIndex(GE::Uint32 uMaxSize);
	~GEIndex(void);

public:
	bool				Insert(GE::Uint32& uID, GE::Uint32& uIdx);				//插入一个元素，返回分配的ID和索引
	bool				Remove(GE::Uint32 uID, GE::Uint32& uIdx);				//删除一个元素，返回删除的索引
	bool				HasID(GE::Uint32 uID, GE::Uint32& uIdx);				//查询某个ID对应的索引
	bool				HasIndex(GE::Uint32& uID, GE::Uint32 uIdx);				//查询某个索引对应的ID
	bool				IterNext(GE::Uint32& uID, GE::Uint32& uIdx);			//迭代下一个元素的ID和索引
	bool				IterPrev(GE::Uint32& uID, GE::Uint32& uIdx);			//迭代上一个元素的ID和索引
	GE::Uint32			Size() {return m_uSize;}								//存储的元素个数
	GE::Uint32			MaxSize() {return m_uMaxSize;}							//最大可存储的元素个数
	GE::Uint32			IdxByID(GE::Uint32 uID) {return uID % MaxSize();}		//根据ID计算索引

private:
	GE::Uint32			m_uSize;
	GE::Uint32			m_uMaxSize;
	GE::Uint32			m_uFreeListIdx;
	GE::Uint32			m_uUserListIdx;

	_GEIIElem*			m_pData;
};

