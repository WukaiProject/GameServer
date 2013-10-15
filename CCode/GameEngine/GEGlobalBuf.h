/************************************************************************
一个全局的buf，注意这个线程不安全!
************************************************************************/
#pragma once
#include <string.h>
#include "GEInteger.h"
#include "GESingleton.h"

class GEGlobalBuf
	: public GESingleton<GEGlobalBuf>
{
	GE_DISABLE_BOJ_CPY(GEGlobalBuf);
public:
	GEGlobalBuf(void) {this->Reset();}
	~GEGlobalBuf(void) {}
public:
	// 重置buf
	void			Reset() {m_uWriteSize = 0; m_uWriteFence = &m_buf[0];}
	// 头指针
	void*			HeadPtr() {return &m_buf[0];}
	// 当前写指针
	void*			NowPtr() {return m_uWriteFence;}
	// 偏移指针（相对于头指针）
	void*			SomePtr(GE::Uint16 uSize) {return &m_buf[uSize];}
	// 最大大小
	GE::Uint16		MaxSize() {return MAX_UINT16;}
	// 已写大小
	GE::Uint16		WriteSize() {return m_uWriteSize;}
	// 可写大小
	GE::Uint16		CanWriteSize() {return MAX_UINT16 - m_uWriteSize;}
	// 写数据（安全）
	bool			WriteBytes(const void* pHead, GE::Uint16 uSize)
	{
		if (uSize < this->CanWriteSize())
		{
			this->WriteBytes_us(pHead, uSize);
			this->MoveWriteFence_us(uSize);
			return true;
		}
		return false;
	}

private:
	// 写数据（不安全）
	void			WriteBytes_us(const void* pHead, GE::Uint16 uSize)
	{
		memcpy(m_uWriteFence, pHead, uSize);
	}
	// 移动写指针（不安全）
	void			MoveWriteFence_us(GE::Uint16 uSize)
	{
		m_uWriteSize += uSize;
		m_uWriteFence = &m_buf[m_uWriteSize];
	}

private:
	GE::Uint16		m_uWriteSize;				//已经写的长度
	void*			m_uWriteFence;				//可写的头指针
	char			m_buf[MAX_UINT16];			//buf数组
};

