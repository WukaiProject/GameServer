/*我是UTF8无签名编码*/
#include "GENetMessage.h"
#include "datetime.h"
#include "GEGlobalBuf.h"
#include "GEIO.h"

#define MAX_STACK_DEEP 30

#define PACK_NEED_SIZE(uSize)	\
	if (HasError())	\
	{	\
		return false;	\
	}	\
	if (uSize > this->CanPackSize())	\
	{	\
		m_bHasError = true;	\
		return false;	\
	}

#define UNPACK_NEED_SIZE(uSize)	\
	if (HasError())	\
	{	\
		return false;	\
	}	\
	if (uSize > this->CanUnpackSize())	\
	{	\
		m_bHasError = true;	\
		return false;	\
	}

/*
各种数据的标识
*/
#define NONE_FLAG				-100			//None
#define TRUE_FLAG				-101			//True
#define FALSE_FLAG				-102			//False
#define SMALL_TUPLE_FLAG		-103			//Tuple
#define BIG_TUPLE_FLAG			-104			//Tuple
#define SMALL_LIST_FLAG			-105			//List
#define BIG_LIST_FLAG			-106			//List
#define SMALL_SET_FLAG			-107			//Set
#define BIG_SET_FLAG			-108			//Set
#define SMALL_DICT_FLAG			-109			//Dict
#define BIG_DICT_FLAG			-110			//Dict
#define SMALL_STRING_FLAG		-111			//String
#define BIG_STRING_FLAG			-112			//String
#define DATETIME_FLAG			-113			//DateTime
#define SIGNED_INT8_FLAG		-114			//signed int8
#define SIGNED_INT16_FLAG		-115			//signed int16
#define SIGNED_INT32_FLAG		-116			//signed int32
#define SIGNED_INT64_FLAG		-117			//signed int64

PackMessage::PackMessage()
{
	this->m_pBufHead = static_cast<char*>(GEGlobalBuf::Instance()->HeadPtr());
	this->m_uBufSize = GEGlobalBuf::Instance()->MaxSize();
	this->Reset();
}

PackMessage::PackMessage( void* pBufHead, GE::Uint16 uBufSize)
{
	this->m_pBufHead = static_cast<char*>(pBufHead);
	this->m_uBufSize = uBufSize;
	this->Reset();
}

PackMessage::~PackMessage( void )
{

}

void PackMessage::Reset()
{
	this->m_pPackFence = this->m_pBufHead;
	this->m_pMsg = NULL;
	this->m_uPackSize = 0;
	this->m_bHasError = false;
}

bool PackMessage::PackMsg( GE::Uint16 uType )
{
	MsgBase msg(uType);
	return this->PackMsg(&msg, msg.Size());
}

bool PackMessage::PackMsg( MsgBase* pMsg, size_t uSize )
{
	return this->PackMsg(pMsg, static_cast<GE::Uint16>(uSize));
}

bool PackMessage::PackMsg( MsgBase* pMsg, GE::Uint16 uSize )
{
	if (HasError())
	{
		return false;
	}
	if (0 != this->PackSize() || uSize > this->CanPackSize())
	{
		m_bHasError = true;
		return false;
	}
	m_pMsg = reinterpret_cast<MsgBase*>(m_pBufHead);
	this->Pack_us(pMsg, uSize);
	m_pMsg->Size(uSize);
	return true;
}

bool PackMessage::PackI8( GE::Int8 i8 )
{
	PACK_NEED_SIZE(sizeof(GE::Int8));
	this->Pack_us(&i8, sizeof(GE::Int8));
	return true;
}

bool PackMessage::PackI16( GE::Int16 i16 )
{
	PACK_NEED_SIZE(sizeof(GE::Int16));
	this->Pack_us(&i16, sizeof(GE::Int16));
	return true;
}

bool PackMessage::PackI32( GE::Int32 i32 )
{
	PACK_NEED_SIZE(sizeof(GE::Int32));
	this->Pack_us(&i32, sizeof(GE::Int32));
	return true;
}

bool PackMessage::PackI64( GE::Int64 i64)
{
	PACK_NEED_SIZE(sizeof(GE::Int64));
	this->Pack_us(&i64, sizeof(GE::Int64));
	return true;
}

bool PackMessage::PackUi8( GE::Uint8 ui8 )
{
	PACK_NEED_SIZE(sizeof(GE::Uint8));
	this->Pack_us(&ui8, sizeof(GE::Uint8));
	return true;
}

bool PackMessage::PackUi16( GE::Uint16 ui16 )
{
	PACK_NEED_SIZE(sizeof(GE::Uint16));
	this->Pack_us(&ui16, sizeof(GE::Uint16));
	return true;
}

bool PackMessage::PackUi32( GE::Uint32 ui32 )
{
	PACK_NEED_SIZE(sizeof(GE::Uint32));
	this->Pack_us(&ui32, sizeof(GE::Uint32));
	return true;
}

bool PackMessage::PackUi64( GE::Uint64 ui64)
{
	PACK_NEED_SIZE(sizeof(GE::Uint64));
	this->Pack_us(&ui64, sizeof(GE::Uint64));
	return true;
}

bool PackMessage::PackStream( const void* pHead, GE::Uint16 uSize )
{
	PACK_NEED_SIZE(uSize);
	this->Pack_us(pHead, uSize);
	return true;
}

bool PackMessage::PackStream_1( const void* pHead, GE::Uint8 uSize )
{
	PACK_NEED_SIZE(uSize + sizeof(GE::Uint8));
	this->Pack_us(&uSize, sizeof(GE::Uint8));
	this->Pack_us(pHead, uSize);
	return true;
}

bool PackMessage::PackStream_2( const void* pHead, GE::Uint16 uSize )
{
	PACK_NEED_SIZE(uSize + sizeof(GE::Uint16));
	this->Pack_us(&uSize, sizeof(GE::Uint16));
	this->Pack_us(pHead, uSize);
	return true;
}

bool PackMessage::PackPyObj( PyObject* pyObj_BorrowRef )
{
	this->m_StackDeep = 0;
	return this->PackPyObj_Help(pyObj_BorrowRef);
}

bool PackMessage::PackIntObj( GE::Int64 i64 )
{
	// 1个字节可以表示
	if (i64 <= MAX_INT8 && i64 >= MIN_INT8)
	{
		GE::Int8 i8 = static_cast<GE::Int8>(i64);
		// 不带标识位
		if (i8 > NONE_FLAG)
		{
			this->PackI8(i8);
		}
		// 要带标识位
		else
		{
			this->PackI8(SIGNED_INT8_FLAG);
			this->PackI8(i8);
		}
	}
	// 2个字节可以表示
	else if (i64 <= MAX_INT16 && i64 >= MIN_INT16)
	{
		GE::Uint16 i16 = static_cast<GE::Uint16>(i64);
		this->PackI8(SIGNED_INT16_FLAG);
		this->PackI16(i16);
	}
	// 4个字节可以表示
	else if (i64 <= MAX_INT32 && i64 >= MIN_INT32)
	{
		GE::Uint32 i32 = static_cast<GE::Uint32>(i64);
		this->PackI8(SIGNED_INT32_FLAG);
		this->PackI32(i32);
	}
	// 8字节可以表示
	else
	{
		this->PackI8(SIGNED_INT64_FLAG);
		this->PackI64(i64);
	}
	return this->HasError();
}

bool PackMessage::PackSpe1( GE::Int64 i64, PyObject* pyObj_BorrowRef )
{
	this->PackI8(SMALL_TUPLE_FLAG);
	this->PackUi8(2);
	this->PackIntObj(i64);
	this->PackPyObj(pyObj_BorrowRef);
	return this->HasError();
}

void PackMessage::Pack_us( const void* pHead, GE::Uint16 uSize )
{
	if (0 != uSize)
	{
		memcpy(m_pPackFence, pHead, uSize);
		m_pPackFence += uSize;
		m_uPackSize += uSize;
		m_pMsg->AddSize(uSize);
	}
}

GE::Uint16 PackMessage::PyObjSize( PyObject* pyObj_BorrowRef, GE::Int32 MaxSize )
{
	GE::Int32 Size = 0;
	if (pyObj_BorrowRef == Py_None)
	{
		return 1;
	}
	else if (pyObj_BorrowRef == Py_True)
	{
		return 1;
	}
	else if (pyObj_BorrowRef == Py_False)
	{
		return 1;
	}
	else if (PyInt_Check(pyObj_BorrowRef) || PyLong_Check(pyObj_BorrowRef))
	{
		GE::Int64 i64 = GEPython::PyObjAsI64(pyObj_BorrowRef);
		// 1个字节可以表示
		if (i64 <= MAX_INT8 && i64 >= MIN_INT8)
		{
			GE::Int8 i8 = static_cast<GE::Int8>(i64);
			// 不带标识位
			if (i8 > NONE_FLAG)
			{
				return 1;
			}
			// 要带标识位
			else
			{
				return 2;
			}
		}
		// 2个字节可以表示
		else if (i64 <= MAX_INT16 && i64 >= MIN_INT16)
		{
			return 3;
		}
		// 4个字节可以表示
		else if (i64 <= MAX_INT32 && i64 >= MIN_INT32)
		{
			return 5;
		}
		// 8字节可以表示
		else
		{
			return 9;
		}
	}
	else if (PyTuple_CheckExact(pyObj_BorrowRef))
	{
		GE::Uint16 uSize = static_cast<GE::Uint16>(PyTuple_Size(pyObj_BorrowRef));
		if (uSize > MAX_UINT8)
		{
			Size += 3;
		}
		else
		{
			Size += 2;
		}
		GE_ITER_UI16(idx, uSize)
		{
			Size += this->PyObjSize(PyTuple_GET_ITEM(pyObj_BorrowRef, idx), MaxSize - Size);
			if (Size > MaxSize)
			{
				return Size;
			}
		}
		return Size;
	}
	else if (PyList_CheckExact(pyObj_BorrowRef))
	{
		GE::Uint16 uSize = static_cast<GE::Uint16>(PyList_Size(pyObj_BorrowRef));
		if (uSize > MAX_UINT8)
		{
			Size += 3;
		}
		else
		{
			Size += 2;
		}
		GE_ITER_UI16(idx, uSize)
		{
			Size += this->PyObjSize(PyList_GET_ITEM(pyObj_BorrowRef, idx), MaxSize - Size);
			if (Size > MaxSize)
			{
				return Size;
			}
		}
		return Size;
	}
	else if (PySet_Check(pyObj_BorrowRef))
	{
		GE::Uint16 uSize = static_cast<GE::Uint16>(PySet_Size(pyObj_BorrowRef));
		if (uSize > MAX_UINT8)
		{
			Size += 3;
		}
		else
		{
			Size += 2;
		}
		if (0 == uSize)
		{
			return Size;
		}
		PyObject* iterator = PyObject_GetIter(pyObj_BorrowRef);
		PyObject* item = NULL;
		GE_ITER_UI16(idx, uSize)
		{
			item = PyIter_Next(iterator);
			GE_ERROR(NULL != item);
			Size += this->PyObjSize(item, MaxSize - Size);
			if (Size > MaxSize)
			{
				return Size;
			}
			Py_DECREF(item);
		}
		Py_DECREF(iterator);
		return Size;
	}
	else if (PyDict_CheckExact(pyObj_BorrowRef))
	{
		GE::Uint16 uSize = static_cast<GE::Uint16>(PyDict_Size(pyObj_BorrowRef));
		if (uSize > MAX_UINT8)
		{
			Size += 3;
		}
		else
		{
			Size += 2;
		}
		Py_ssize_t pos = 0;
		PyObject* key = NULL;
		PyObject* value = NULL;
		GE_ITER_UI16(idx, uSize)
		{
			int ret = PyDict_Next(pyObj_BorrowRef, &pos, &key, &value);
			GE_ERROR(0 != ret);
			Size += this->PyObjSize(key, MaxSize - Size);
			Size += this->PyObjSize(value, MaxSize - Size);
			if (Size > MaxSize)
			{
				return Size;
			}
		}
		return Size;
	}
	else if (PyString_CheckExact(pyObj_BorrowRef))
	{
		GE::Uint16 uSize = static_cast<GE::Uint16>(PyString_Size(pyObj_BorrowRef));
		const void* pHead = PyString_AS_STRING(pyObj_BorrowRef);
		if (uSize > MAX_UINT8)
		{
			return uSize + 3;
		}
		else
		{
			return uSize + 2;
		}
	}
	else if (GEPython::PyDateTimeCheck(pyObj_BorrowRef))
	{
		return 13;
	}
	else
	{
		GE_EXC<<"can only pack size int long string tuple list set dict."<<GE_END;
		return MAX_UINT16;
	}
	return MAX_UINT16;
}

bool PackMessage::PackPyObj_Help( PyObject* pyObj_BorrowRef )
{
	// Python对象会循环引用，故要检测递归调用层数
	if (this->m_StackDeep >= MAX_STACK_DEEP)
	{
		GE_EXC<<"pack obj stack overflow."<<GE_END;
		this->SetError();
		return false;
	}
	++ this->m_StackDeep;

	if (!this->HasError())
	{
		if (NULL == pyObj_BorrowRef)
		{
			GE_EXC<<"pyObj is NULL on PackPyObj_Help."<<GE_END;
		}
		if (pyObj_BorrowRef == Py_None)
		{
			this->PackI8(NONE_FLAG);
		}
		else if (pyObj_BorrowRef == Py_True)
		{
			this->PackI8(TRUE_FLAG);
		}
		else if (pyObj_BorrowRef == Py_False)
		{
			this->PackI8(FALSE_FLAG);
		}
		else if (PyInt_Check(pyObj_BorrowRef) || PyLong_Check(pyObj_BorrowRef))
		{
			GE::Int64 i64 = GEPython::PyObjAsI64(pyObj_BorrowRef);
			// 1个字节可以表示
			if (i64 <= MAX_INT8 && i64 >= MIN_INT8)
			{
				GE::Int8 i8 = static_cast<GE::Int8>(i64);
				// 不带标识位
				if (i8 > NONE_FLAG)
				{
					this->PackI8(i8);
				}
				// 要带标识位
				else
				{
					this->PackI8(SIGNED_INT8_FLAG);
					this->PackI8(i8);
				}
			}
			// 2个字节可以表示
			else if (i64 <= MAX_INT16 && i64 >= MIN_INT16)
			{
				GE::Uint16 i16 = static_cast<GE::Uint16>(i64);
				this->PackI8(SIGNED_INT16_FLAG);
				this->PackI16(i16);
			}
			// 4个字节可以表示
			else if (i64 <= MAX_INT32 && i64 >= MIN_INT32)
			{
				GE::Uint32 i32 = static_cast<GE::Uint32>(i64);
				this->PackI8(SIGNED_INT32_FLAG);
				this->PackI32(i32);
			}
			// 8字节可以表示
			else
			{
				this->PackI8(SIGNED_INT64_FLAG);
				this->PackI64(i64);
			}
		}
		else if (PyTuple_Check(pyObj_BorrowRef))
		{
			GE::Uint16 uSize = static_cast<GE::Uint16>(PyTuple_Size(pyObj_BorrowRef));
			if (uSize > MAX_UINT8)
			{
				this->PackI8(BIG_TUPLE_FLAG);
				this->PackUi16(uSize);
			}
			else
			{
				this->PackI8(SMALL_TUPLE_FLAG);
				this->PackUi8(static_cast<GE::Uint8>(uSize));
			}
			GE_ITER_UI16(idx, uSize)
			{
				this->PackPyObj_Help(PyTuple_GET_ITEM(pyObj_BorrowRef, idx));
			}
		}
		else if (PyList_Check(pyObj_BorrowRef))
		{
			GE::Uint16 uSize = static_cast<GE::Uint16>(PyList_Size(pyObj_BorrowRef));
			if (uSize > MAX_UINT8)
			{
				this->PackI8(BIG_LIST_FLAG);
				this->PackUi16(uSize);
			}
			else
			{
				this->PackI8(SMALL_LIST_FLAG);
				this->PackUi8(static_cast<GE::Uint8>(uSize));
			}
			GE_ITER_UI16(idx, uSize)
			{
				this->PackPyObj_Help(PyList_GET_ITEM(pyObj_BorrowRef, idx));
			}
		}
		else if (PySet_Check(pyObj_BorrowRef))
		{
			GE::Uint16 uSize = static_cast<GE::Uint16>(PySet_Size(pyObj_BorrowRef));
			if (uSize > MAX_UINT8)
			{
				this->PackI8(BIG_SET_FLAG);
				this->PackUi16(uSize);
			}
			else
			{
				this->PackI8(SMALL_SET_FLAG);
				this->PackUi8(static_cast<GE::Uint8>(uSize));
			}
			if (0 != uSize)
			{
				PyObject* iterator = PyObject_GetIter(pyObj_BorrowRef);
				PyObject* item = NULL;
				GE_ITER_UI16(idx, uSize)
				{
					item = PyIter_Next(iterator);
					GE_ERROR(NULL != item);
					this->PackPyObj_Help(item);
					Py_DECREF(item);
				}
				Py_DECREF(iterator);
			}
		}
		else if (PyDict_Check(pyObj_BorrowRef))
		{
			GE::Uint16 uSize = static_cast<GE::Uint16>(PyDict_Size(pyObj_BorrowRef));
			if (uSize > MAX_UINT8)
			{
				this->PackI8(BIG_DICT_FLAG);
				this->PackUi16(uSize);
			}
			else
			{
				this->PackI8(SMALL_DICT_FLAG);
				this->PackUi8(static_cast<GE::Uint8>(uSize));
			}
			Py_ssize_t pos = 0;
			PyObject* key = NULL;
			PyObject* value = NULL;
			GE_ITER_UI16(idx, uSize)
			{
				int ret = PyDict_Next(pyObj_BorrowRef, &pos, &key, &value);
				GE_ERROR(0 != ret);
				this->PackPyObj_Help(key);
				this->PackPyObj_Help(value);
			}
		}
		else if (PyString_CheckExact(pyObj_BorrowRef))
		{
			GE::Uint16 uSize = static_cast<GE::Uint16>(PyString_Size(pyObj_BorrowRef));
			const void* pHead = PyString_AS_STRING(pyObj_BorrowRef);
			if (uSize > MAX_UINT8)
			{
				this->PackI8(BIG_STRING_FLAG);
				this->PackStream_2(pHead, uSize);
			}
			else
			{
				this->PackI8(SMALL_STRING_FLAG);
				this->PackStream_1(pHead, static_cast<GE::Uint8>(uSize));
			}
		}
		else if (GEPython::PyDateTimeCheck(pyObj_BorrowRef))
		{
			PyDateTime_Date* p = reinterpret_cast<PyDateTime_Date*>(pyObj_BorrowRef);
			this->PackI8(DATETIME_FLAG);
			this->PackUi16(static_cast<GE::Uint16>(PyDateTime_GET_YEAR(p)));
			this->PackUi16(static_cast<GE::Uint16>(PyDateTime_GET_MONTH(p)));
			this->PackUi16(static_cast<GE::Uint16>(PyDateTime_GET_DAY(p)));
			this->PackUi16(static_cast<GE::Uint16>(PyDateTime_DATE_GET_HOUR(p)));
			this->PackUi16(static_cast<GE::Uint16>(PyDateTime_DATE_GET_MINUTE(p)));
			this->PackUi16(static_cast<GE::Uint16>(PyDateTime_DATE_GET_SECOND(p)));
		}
		else
		{
			GE_EXC<<"can only pack int long string tuple list set dict."<<GE_END;
			this->SetError();
		}
	}
	
	--this->m_StackDeep;
	return this->HasError();
}

bool PackMessage::Assign( const void* pHead, GE::Uint16 uSize )
{
	this->Reset();
	if (uSize > this->CanPackSize())
	{
		GE_EXC<<"assign msg size too long."<<GE_END;
		this->m_bHasError = true;
		return false;
	}
	// 注意这里不能使用Pack_us, 因为不要修正消息长度
	memcpy(m_pPackFence, pHead, uSize);
	m_pPackFence += uSize;
	m_uPackSize += uSize;
	// 这里保险起见，检测下消息长度
	if (this->m_pMsg->Size() != uSize)
	{
		GE_EXC<<"assign msg size error."<<GE_END;
		this->m_bHasError = true;
		return false;
	}
	return true;
}

UnpackMessage::UnpackMessage( MsgBase* pMsg )
	: m_pMsg(pMsg)
	, m_bHasError(false)
{
	m_pUnpackFence = reinterpret_cast<char*>(m_pMsg);
	m_uUnpackSize = 0;
}

UnpackMessage::~UnpackMessage()
{

}

bool UnpackMessage::UnpackMsg( size_t uSize )
{
	return this->UnpackMsg(static_cast<GE::Uint16>(uSize));
}

bool UnpackMessage::UnpackMsg( GE::Uint16 uSize )
{
	if (0 != UnpackSize() || uSize > this->CanUnpackSize())
	{
		return false;
	}
	m_pUnpackFence += uSize;
	m_uUnpackSize += uSize;
	return true;
}

bool UnpackMessage::UnpackI8( GE::Int8& i8 )
{
	UNPACK_NEED_SIZE(sizeof(GE::Int8));
	i8 = *(reinterpret_cast<GE::Int8*>(m_pUnpackFence));
	m_pUnpackFence += sizeof(GE::Int8);
	m_uUnpackSize += sizeof(GE::Int8);
	return true;
}

bool UnpackMessage::UnpackI16( GE::Int16& i16 )
{
	UNPACK_NEED_SIZE(sizeof(GE::Int16));
	i16 = *(reinterpret_cast<GE::Int16*>(m_pUnpackFence));
	m_pUnpackFence += sizeof(GE::Int16);
	m_uUnpackSize += sizeof(GE::Int16);
	return true;
}

bool UnpackMessage::UnpackI32( GE::Int32& i32 )
{
	UNPACK_NEED_SIZE(sizeof(GE::Int32));
	i32 = *(reinterpret_cast<GE::Int32*>(m_pUnpackFence));
	m_pUnpackFence += sizeof(GE::Int32);
	m_uUnpackSize += sizeof(GE::Int32);
	return true;
}

bool UnpackMessage::UnpackI64( GE::Int64& i64 )
{
	UNPACK_NEED_SIZE(sizeof(GE::Int64));
	i64 = *(reinterpret_cast<GE::Int64*>(m_pUnpackFence));
	m_pUnpackFence += sizeof(GE::Int64);
	m_uUnpackSize += sizeof(GE::Int64);
	return true;
}

bool UnpackMessage::UnpackUi8( GE::Uint8& ui8 )
{
	UNPACK_NEED_SIZE(sizeof(GE::Uint8));
	ui8 = *(reinterpret_cast<GE::Uint8*>(m_pUnpackFence));
	m_pUnpackFence += sizeof(GE::Uint8);
	m_uUnpackSize += sizeof(GE::Uint8);
	return true;
}

bool UnpackMessage::UnpackUi16( GE::Uint16& ui16 )
{
	UNPACK_NEED_SIZE(sizeof(GE::Uint16));
	ui16 = *(reinterpret_cast<GE::Uint16*>(m_pUnpackFence));
	m_pUnpackFence += sizeof(GE::Uint16);
	m_uUnpackSize += sizeof(GE::Uint16);
	return true;
}

bool UnpackMessage::UnpackUi32( GE::Uint32& ui32 )
{
	UNPACK_NEED_SIZE(sizeof(GE::Uint32));
	ui32 = *(reinterpret_cast<GE::Uint32*>(m_pUnpackFence));
	m_pUnpackFence += sizeof(GE::Uint32);
	m_uUnpackSize += sizeof(GE::Uint32);
	return true;
}

bool UnpackMessage::UnpackUi64( GE::Uint64& ui64 )
{
	UNPACK_NEED_SIZE(sizeof(GE::Uint64));
	ui64 = *(reinterpret_cast<GE::Uint64*>(m_pUnpackFence));
	m_pUnpackFence += sizeof(GE::Uint64);
	m_uUnpackSize += sizeof(GE::Uint64);
	return true;
}

bool UnpackMessage::UnpackStream( void** pHead, GE::Uint16 uSize )
{
	UNPACK_NEED_SIZE(uSize);
	*pHead = m_pUnpackFence;
	m_pUnpackFence += uSize;
	m_uUnpackSize += uSize;
	return true;
}

bool UnpackMessage::UnpackStream_1( void** pHead, GE::Uint8& uSize )
{
	if (HasError())
	{
		return false;
	}
	if (!this->UnpackUi8(uSize))
	{
		m_bHasError = true;
		return false;
	}
	if (uSize > this->CanUnpackSize())
	{
		m_bHasError = true;
		return false;
	}
	*pHead = m_pUnpackFence;
	m_pUnpackFence += uSize;
	m_uUnpackSize += uSize;
	return true;
}

bool UnpackMessage::UnpackStream_2( void** pHead, GE::Uint16& uSize )
{
	if (HasError())
	{
		return false;
	}
	if (!this->UnpackUi16(uSize))
	{
		m_bHasError = true;
		return false;
	}
	if (uSize > this->CanUnpackSize())
	{
		m_bHasError = true;
		return false;
	}
	*pHead = m_pUnpackFence;
	m_pUnpackFence += uSize;
	m_uUnpackSize += uSize;
	return true;
}

bool UnpackMessage::UnpackPyObj( GEPython::Object& pyObj )
{
	if (this->CanUnpackSize() == 0)
	{
		pyObj.SetObj_BorrowRef(Py_None);
		return true;
	}
	pyObj.SetObj_NewRef(this->UnpackPyObj_Help());
#if WIN
	if (this->CanUnpackSize() > 3)
	{
		GE_EXC<<"has bytes("<<this->CanUnpackSize()<<") not deal."<<GE_END;
	}
#endif
	if (this->HasError())
	{
		GE_EXC<<"unpack python data has error."<<GE_END;
		return false;
	}
	else
	{
		return true;
	}
}

bool UnpackMessage::UnpackIntObj( GE::Int64& i64 )
{
	// 解包Int
	GE::Int8 flag = 0;
	this->UnpackI8(flag);
	switch (flag)
	{
	case SIGNED_INT8_FLAG:
		{
			GE::Int8 i8 = 0;
			if (!this->UnpackI8(i8))
			{
				return false;
			}
			i64 = i8;
			break;
		}
	case SIGNED_INT16_FLAG:
		{
			GE::Int16 i16 = 0;
			if (!this->UnpackI16(i16))
			{
				return false;
			}
			i64 = i16;
			break;
		}
	case SIGNED_INT32_FLAG:
		{
			GE::Int32 i32 = 0;
			if (!this->UnpackI32(i32))
			{
				return false;
			}
			i64 = i32;
			break;
		}
	case SIGNED_INT64_FLAG:
		{
			if (!this->UnpackI64(i64))
			{
				return false;
			}
			break;
		}
	default:
		{
			this->HasError();
			GE_EXC<<"unpack int obj error."<<GE_END;
			return false;
		}
	}
	// 不会到这里来的
	this->HasError();
	return false;
}

bool UnpackMessage::UnpackSpe1( GE::Int64& i64, GEPython::Object& pyObj )
{
	// 检测Tuple标识
	GE::Int8 flag = 0;
	this->UnpackI8(flag);
	if (flag != SMALL_TUPLE_FLAG)
	{
		GE_EXC<<"unpack spe1 error because flag("<<flag<<") is not SMALL_TUPLE_FLAG."<<GE_END;
		this->HasError();
		return false;
	}
	// 检测Tuple长度
	GE::Uint8 uSize = 0;
	this->UnpackUi8(uSize);
	if (uSize != 2)
	{
		GE_EXC<<"unpack spe1 error because uSize("<<uSize<<") is not 2."<<GE_END;
		this->HasError();
		return false;
	}
	// 解包Int
	this->UnpackIntObj(i64);
	// 解包Python对象
	return this->UnpackPyObj(pyObj);
}

PyObject* UnpackMessage::UnpackPyObj_Help()
{
	GE::Int8 flag = 0;
	this->UnpackI8(flag);
	// 是小数值，直接返回
	if (flag > NONE_FLAG)
	{
		return GEPython::PyObjFromI32(flag);
	}
	switch(flag)
	{
	case NONE_FLAG:
		{
			Py_RETURN_NONE;
			break;
		}
	case TRUE_FLAG:
		{
			Py_RETURN_TRUE;
			break;
		}
	case FALSE_FLAG:
		{
			Py_RETURN_FALSE;
			break;
		}
	case SMALL_TUPLE_FLAG:
		{
			GE::Uint8 usize_8 = 0;
			this->UnpackUi8(usize_8);
			GEPython::Tuple tup(usize_8);
			GE_ITER_UI8(idx, usize_8)
			{
				tup.AppendObj_NewRef(this->UnpackPyObj_Help());
				if (this->HasError()) {Py_RETURN_NONE;}
			}
			return tup.GetTuple_NewRef();
		}
	case BIG_TUPLE_FLAG:
		{
			GE::Uint16 usize_16 = 0;
			this->UnpackUi16(usize_16);
			GEPython::Tuple tup(usize_16);
			GE_ITER_UI16(idx, usize_16)
			{
				tup.AppendObj_NewRef(this->UnpackPyObj_Help());
				if (this->HasError()) {Py_RETURN_NONE;}
			}
			return tup.GetTuple_NewRef();
		}
	case SMALL_LIST_FLAG:
		{
			GE::Uint8 usize_8 = 0;
			this->UnpackUi8(usize_8);
			GEPython::List lis;
			GE_ITER_UI8(idx, usize_8)
			{
				lis.AppendObj_NewRef(this->UnpackPyObj_Help());
				if (this->HasError()) {Py_RETURN_NONE;}
			}
			return lis.GetList_NewRef();
		}
	case BIG_LIST_FLAG:
		{
			GE::Uint16 usize_16 = 0;
			this->UnpackUi16(usize_16);
			GEPython::List lis;
			GE_ITER_UI16(idx, usize_16)
			{
				lis.AppendObj_NewRef(this->UnpackPyObj_Help());
				if (this->HasError()) {Py_RETURN_NONE;}
			}
			return lis.GetList_NewRef();
		}
	case SMALL_SET_FLAG:
		{
			GE::Uint8 usize_8 = 0;
			this->UnpackUi8(usize_8);
			GEPython::Set set;
			GE_ITER_UI8(idx, usize_8)
			{
				set.AddObj_NewRef(this->UnpackPyObj_Help());
				if (this->HasError()) {Py_RETURN_NONE;}
			}
			return set.GetSet_NewRef();
		}
	case BIG_SET_FLAG:
		{
			GE::Uint16 usize_16 = 0;
			this->UnpackUi16(usize_16);
			GEPython::Set set;
			GE_ITER_UI16(idx, usize_16)
			{
				set.AddObj_NewRef(this->UnpackPyObj_Help());
				if (this->HasError()) {Py_RETURN_NONE;}
			}
			return set.GetSet_NewRef();
		}
	case SMALL_DICT_FLAG:
		{
			GE::Uint8 usize_8 = 0;
			this->UnpackUi8(usize_8);
			GEPython::Dict dic;
			GE_ITER_UI8(idx, usize_8)
			{
				PyObject* pyKeyNewRef = this->UnpackPyObj_Help();
				PyObject* pyValueNewRef = this->UnpackPyObj_Help();
				dic.SetObj_NewRef(pyKeyNewRef, pyValueNewRef);
				if (this->HasError()) {Py_RETURN_NONE;}
			}
			return dic.GetDict_NewRef();
		}
	case BIG_DICT_FLAG:
		{
			GE::Uint16 usize_16 = 0;
			this->UnpackUi16(usize_16);
			GEPython::Dict dic;
			GE_ITER_UI16(idx, usize_16)
			{
				PyObject* pyKeyNewRef = this->UnpackPyObj_Help();
				PyObject* pyValueNewRef = this->UnpackPyObj_Help();
				dic.SetObj_NewRef(pyKeyNewRef, pyValueNewRef);
				if (this->HasError()) {Py_RETURN_NONE;}
			}
			return dic.GetDict_NewRef();
		}
	case SMALL_STRING_FLAG:
		{
			GE::Uint8 usize_8 = 0;
			void* pHead = NULL;
			if (this->UnpackStream_1(&pHead, usize_8))
			{
				return PyString_FromStringAndSize(static_cast<char*>(pHead), usize_8);
			}
			else
			{
				Py_RETURN_NONE;
			}
		}
	case BIG_STRING_FLAG:
		{
			GE::Uint16 usize_16 = 0;
			void* pHead = NULL;
			if (this->UnpackStream_2(&pHead, usize_16))
			{
				return PyString_FromStringAndSize(static_cast<char*>(pHead), usize_16);
			}
			else
			{
				Py_RETURN_NONE;
			}
		}
	case DATETIME_FLAG:
		{
			GE::Uint16 y = 2012;
			GE::Uint16 m = 1;
			GE::Uint16 d = 1;
			GE::Uint16 H = 0;
			GE::Uint16 M = 0;
			GE::Uint16 S = 0;
			this->UnpackUi16(y);
			this->UnpackUi16(m);
			this->UnpackUi16(d);
			this->UnpackUi16(H);
			this->UnpackUi16(M);
			this->UnpackUi16(S);
			/*
			Return value: New reference.
			*/
			PyObject* p = GEPython::PyObjFromDatetime(y, m, d, H, M, S);
			if (p)
			{
				return p;
			}
			else
			{
				Py_RETURN_NONE;
			}
		}
	case SIGNED_INT8_FLAG:
		{
			GE::Int8 i8 = 0;
			if (this->UnpackI8(i8))
			{
				return GEPython::PyObjFromI32(i8);
			}
			else
			{
				Py_RETURN_NONE;
			}
		}
	case SIGNED_INT16_FLAG:
		{
			GE::Int16 i16 = 0;
			if (this->UnpackI16(i16))
			{
				return GEPython::PyObjFromI32(i16);
			}
			else
			{
				Py_RETURN_NONE;
			}
		}
	case SIGNED_INT32_FLAG:
		{
			GE::Int32 i32 = 0;
			if (this->UnpackI32(i32))
			{
				return GEPython::PyObjFromI32(i32);
			}
			else
			{
				Py_RETURN_NONE;
			}
		}
	case SIGNED_INT64_FLAG:
		{
			GE::Int64 i64 = 0;
			if (this->UnpackI64(i64))
			{
				return GEPython::PyObjFromI64(i64);
			}
			else
			{
				Py_RETURN_NONE;
			}
		}
	default:
		{
			GE_EXC<<"unknown flag("<<static_cast<GE::Int16>(flag)<<")."<<GE_END;
			Py_RETURN_NONE;
		}
	}
	Py_RETURN_NONE;
}

