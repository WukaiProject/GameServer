/************************************************************************
网络层消息定义以及消息结构的一些约束规则。
************************************************************************/
#pragma once
#include "GEInteger.h"
#include "GESingleton.h"
#include "GESingleton.h"
#include "GEPython.h"

// 网络消息定义的宏
#define MSG_BEGIN(className, msgType)	\
class className : public MsgBase	\
{	\
public:	\
	className() : MsgBase(sizeof(className), msgType) {}

// 网络消息最大长度
#define MSG_MAX_SIZE	MAX_UINT16
// 网络消息的基本长度
#define MSG_BASE_SIZE	4

// 基本消息
class MsgBase
{
public:
	MsgBase() : m_uSize(sizeof(MsgBase)), m_uType(0) {}
	MsgBase(GE::Uint16 uType) : m_uSize(sizeof(MsgBase)), m_uType(uType) {}
	MsgBase(GE::Uint16 uSize, GE::Uint16 uType) : m_uSize(uSize), m_uType(uType) {}
	GE::Uint16		Size() const {return m_uSize;}									//消息长度
	void			Size(GE::Uint16 uSize) {m_uSize = uSize;}						//设置消息长度
	void			AddSize(GE::Uint16 uSize) {m_uSize += uSize;}					//增加消息长度
	GE::Uint16		Type() const {return m_uType;}									//消息类型
	void			Type(GE::Uint16 uType) {m_uType = uType;}						//设置消息类型
	void*			Body() {return (char*)this + sizeof(MsgBase);}					//消息体
	GE::Uint16		BodySize() {return m_uSize - sizeof(MsgBase);}					//消息体长度
	/*
	将消息长度修正为MSG_BASE_SIZE的倍数
	以方便在存储的时候使得每条消息字节对齐
	*/
	void			Align()
	{
		GE::Uint16 uDiff = this->m_uSize % MSG_BASE_SIZE;
		if (uDiff)
		{
			this->m_uSize = this->m_uSize - uDiff + MSG_BASE_SIZE;
		}
	}

protected:
	GE::Uint16		m_uSize;
	GE::Uint16		m_uType;
};

class PackMessage
	: public GESingleton<PackMessage>
{
	GE_DISABLE_BOJ_CPY(PackMessage);
public:
	PackMessage();
	PackMessage(void* pBufHead, GE::Uint16 uBufSize);
	~PackMessage(void);
public:
	void				Reset();
	// 首先必须要打包一个消息结构体，后续打包中会自动添加消息头中的长度描述
	bool				PackMsg(GE::Uint16 uType);									//打包消息
	bool				PackMsg(MsgBase* pMsg, size_t uSize);						//打包消息
	bool				PackMsg(MsgBase* pMsg, GE::Uint16 uSize);					//打包消息
	bool				PackI8(GE::Int8 i8);										//打包一个I8
	bool				PackI16(GE::Int16 i16);										//打包一个I16
	bool				PackI32(GE::Int32 i32);										//打包一个I32
	bool				PackI64(GE::Int64 i64);										//打包一个I64
	bool				PackUi8(GE::Uint8 ui8);										//打包一个UI8
	bool				PackUi16(GE::Uint16 ui16);									//打包一个UI16
	bool				PackUi32(GE::Uint32 ui32);									//打包一个UI32
	bool				PackUi64(GE::Uint64 ui64);									//打包一个UI64
	bool				PackStream(const void* pHead, GE::Uint16 uSize);			//打包一个字节流，这个字节流是定长的
	bool				PackStream_1(const void* pHead, GE::Uint8 uSize);			//打包一个字节流，这个字节流是不定长的，先用1个字节描述字节流长度，然后打包字节流。
	bool				PackStream_2(const void* pHead, GE::Uint16 uSize);			//打包一个字节流，这个字节流是不定长的，先用2个字节描述字节流长度，然后打包字节流。
	bool				PackPyObj(PyObject* pyObj_BorrowRef);						//打包一个Python对象
	bool				PackIntObj(GE::Int64 i64);									//将一个Int按照Python格式打包（用于压缩传输数据）
	bool				PackSpe1(GE::Int64 i64, PyObject* pyObj_BorrowRef);			//将两个参数作为Tuple打包（用于RPC调用时打包由C生成的函数ID）
	bool				HasError() {return m_bHasError;}							//打包过程中是否有错误
	void				SetError() {m_bHasError = true;}							//设置打包过程中有错误
	MsgBase*			Msg() {return (MsgBase*)(m_pBufHead);}						//获取消息头
	GE::Uint16			PyObjSize(PyObject* pyObj_BorrowRef, GE::Int32 MaxSize);	//计算打包一个Python对象所需的长度
	void*				GetHead() {return m_pBufHead;}								//获取消息头指针
	GE::Uint16			GetSize() {return this->m_uPackSize;}						//获取消息长度
	void*				GetBodyHead() {return m_pBufHead + sizeof(MsgBase);}		//获取消息体的头指针
	GE::Uint16			GetBodySize() {return m_uPackSize - sizeof(MsgBase);}		//获取消息体的长度
	bool				Assign(const void* pHead, GE::Uint16 uSize);				//填充消息
private:
	inline GE::Uint16	PackSize() {return m_uPackSize;}							//已经打包的长度
	inline GE::Uint16	CanPackSize() {return m_uBufSize - m_uPackSize;}			//还可以打包的长度
	inline void			Pack_us(const void* pHead, GE::Uint16 uSize);				//打包字节流（不安全）
	bool				PackPyObj_Help(PyObject* pyObj_BorrowRef);					//打包一个Python对象（辅助）

private:
	bool				m_bHasError;
	GE::Uint16			m_uBufSize;
	GE::Uint16			m_uPackSize;
	char*				m_pBufHead;
	char*				m_pPackFence;
	MsgBase*			m_pMsg;
	GE::Uint16			m_StackDeep;
};

class UnpackMessage
{
	GE_DISABLE_BOJ_CPY(UnpackMessage);
public:
	// 解包类对象的构建需要一个网络消息
	UnpackMessage(MsgBase* pMsg);
	~UnpackMessage();

public:
	// 从网络消息中解析出一个消息结构体
	bool				UnpackMsg(size_t uSize);									//解包消息
	bool				UnpackMsg(GE::Uint16 uSize);								//解包消息
	bool				UnpackI8(GE::Int8& i8);										//解包一个I8
	bool				UnpackI16(GE::Int16& i16);									//解包一个I16
	bool				UnpackI32(GE::Int32& i32);									//解包一个I32
	bool				UnpackI64(GE::Int64& i64);									//解包一个I64
	bool				UnpackUi8(GE::Uint8& ui8);									//解包一个UI8
	bool				UnpackUi16(GE::Uint16& ui16);								//解包一个UI16
	bool				UnpackUi32(GE::Uint32& ui32);								//解包一个UI32
	bool				UnpackUi64(GE::Uint64& ui64);								//解包一个UI64
	bool				UnpackStream(void** pHead, GE::Uint16 uSize);				//解包一个定长字节流
	bool				UnpackStream_1(void** pHead, GE::Uint8& uSize);				//解包一个不定长字节流。先读取1个字节作为字节流的长度，再解包该长度的字节流
	bool				UnpackStream_2(void** pHead, GE::Uint16& uSize);			//解包一个不定长字节流。先读取2个字节作为字节流的长度，在解包该长度的字节流
	bool				UnpackPyObj(GEPython::Object& pyObj);						//解包一个Python对象
	bool				UnpackIntObj(GE::Int64& i64);								//将数据以Int方式解析（和上面的打包对应）
	bool				UnpackSpe1(GE::Int64& i64, GEPython::Object& pyObj);		//将两个参数以Tuple解包包（用于RPC调用时解包由C生成的函数ID）
	bool				HasError() {return m_bHasError;}							//解包过程是否有错误
	void				SetError() {m_bHasError = true;}							//设置解包出错
	MsgBase*			Msg() {return m_pMsg;}										//获取消息

private:
	GE::Uint16			UnpackSize() {return m_uUnpackSize;}						//解包长度
	GE::Uint16			CanUnpackSize() {return m_pMsg->Size() - m_uUnpackSize;}	//还可以解包的长度
	PyObject*			UnpackPyObj_Help();											//解包一个Python对象（辅助）

private:
	bool				m_bHasError;
	GE::Uint16			m_uUnpackSize;
	char*				m_pUnpackFence;
	MsgBase*			m_pMsg;
};


// 确保MsgBase的长度是4的倍数
GE_STATIC_ASSERT(sizeof(MsgBase) % 4 == 0);

