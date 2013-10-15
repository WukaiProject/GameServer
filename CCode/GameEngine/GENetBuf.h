/************************************************************************
网络消息的基本buf
************************************************************************/
#pragma once
#include "GEInteger.h"
#include "GEDefine.h"

class GENetBuf
{
	GE_DISABLE_BOJ_CPY(GENetBuf);
public:
	GENetBuf(GE::Uint16 uMaxSize);
	~GENetBuf(void);

public:
	void			Reset(GE::Uint16 uSize = 0) {m_uReadSize = uSize; m_uWriteSize = uSize;}//初始化buf内部状态
	void*			HeadPtr(GE::Uint16 uSize = 0) {return m_pHead + uSize;}					//buf的头指针
	GE::Uint16		MaxSize() {return m_uMaxSize;}											//buf或内存块的最大大小
	GE::Uint16		CanWriteSize() {return m_uMaxSize - m_uWriteSize;}						//buf还可以写的空间大小
	GE::Uint16		CanReadSize() {return m_uWriteSize - m_uReadSize;}						//buf还可以读得空间大小
	// 读写buf函数簇
	void*			WriteFence_us();											//buf的当前写指针（没做边界检查，不安全）
	void			WriteBytes_us(const void* pHead, GE::Uint16 uSize);			//将uSize长度的字节拷贝到buf中（没做边界检查，不安全）
	void			MoveWriteFence_us(GE::Uint16 uSize);						//将写指针移动uSize（没做边界检查，不安全）
	void*			ReadFence_us();												//buf的当前的读指针（没做边界检查，不安全）
	void			MoveReadFence_us(GE::Uint16 uSize);							//将读指针移动uSize（没做边界检查，不安全）
	void			InsertBytes_us(const void* pHead, GE::Uint16 uSize);		//将uSize长度的字节拷贝到buf中（重buf头开始，没做边界检查，不安全）

private:
	char*			m_pHead;			//buf的头指针
	GE::Uint16		m_uReadSize;		//已经读的长度
	GE::Uint16		m_uWriteSize;		//已经写的长度
	GE::Uint16		m_uMaxSize;			//buf可存储的最大大小
};

