/************************************************************************
游戏引擎中的跨平台整数定义相关
************************************************************************/
#pragma once
#include "GEDefine.h"

// 定义整数的范围
#define MIN_INT8				(-127 - 1)
#define	MAX_INT8				127
#define MIN_INT16				(-32767 - 1)
#define MAX_INT16				32767
#define MIN_INT32				(-2147483647 - 1)
#define MAX_INT32				2147483647
#define MIN_INT64				(-9223372036854775807 - 1)
#define MAX_INT64				9223372036854775807
#define MAX_UINT8				255U
#define MAX_UINT16				65535U
#define MAX_UINT32				4294967295U
#define MAX_UINT64				18446744073709551615U

// 跨平台整数定义
namespace GE
{
#ifdef WIN
	typedef signed char			Int8;
	typedef signed short		Int16;
	typedef signed int			Int32;
	typedef signed __int64		Int64;
	typedef unsigned char		Uint8;
	typedef unsigned short		Uint16;
	typedef unsigned int		Uint32;
	typedef unsigned __int64	Uint64;
#elif LINUX
	typedef signed char			Int8;
	typedef signed short		Int16;
	typedef signed int			Int32;
	typedef signed long long	Int64;
	typedef unsigned char		Uint8;
	typedef unsigned short		Uint16;
	typedef unsigned int		Uint32;
	typedef unsigned long long	Uint64;
#endif

	// 基于字节定义的整数
	union B2
	{
	public:
		// 构造函数簇
		B2() : _ui16(0) {}
		B2(Uint16 ui16) : _ui16(ui16) {}
		B2(Uint8 ui8_0, Uint8 ui8_1) {_ui8[0] = ui8_0; _ui8[1] = ui8_1;}
		// 反转函数簇
		operator GE::Uint16() const {return _ui16;}
		// 内部引用函数
		Int8&		I8_0() {return _i8[0];}
		Int8&		I8_1() {return _i8[1];}
		Uint8&		UI8_0() {return _ui8[0];}
		Uint8&		UI8_1() {return _ui8[1];}
		Int16&		I16() {return _i16;}
		Uint16&		UI16() {return _ui16;}
	private:
		Uint16		_ui16;
		Int16		_i16;
		Uint8		_ui8[2];
		Int8		_i8[2];
	};

	union B4
	{
	public:
		// 构造函数簇
		B4 () :_ui32(0) {}
		B4 (Uint32 ui32) :_ui32(ui32) {}
		B4 (Uint16 ui16_0, Uint16 ui16_1) {_ui16[0] = ui16_0; _ui16[1] = ui16_1;}
		// 反转函数簇
		operator GE::Uint32() const {return _ui32;}
		// 内部引用函数
		Int8&		I8_0() {return _i8[0];}
		Int8&		I8_1() {return _i8[1];}
		Int8&		I8_2() {return _i8[2];}
		Int8&		I8_3() {return _i8[3];}
		Uint8&		UI8_0() {return _ui8[0];}
		Uint8&		UI8_1() {return _ui8[1];}
		Uint8&		UI8_2() {return _ui8[2];}
		Uint8&		UI8_3() {return _ui8[3];}
		Int16&		I16_0() {return _i16[0];}
		Int16&		I16_1() {return _i16[1];}
		Uint16&		UI16_0() {return _ui16[0];}
		Uint16&		UI16_1() {return _ui16[1];}
		Int32&		I32() {return _i32;}
		Uint32&		UI32() {return _ui32;}
	private:
		Uint32		_ui32;
		Int32		_i32;
		Int8		_i8[4];
		Uint8		_ui8[4];
		Int16		_i16[2];
		Uint16		_ui16[2];
	};
	
	union B8
	{
	public:
		// 构造函数簇
		B8 () : _ui64(0) {_ui64 = 0;}
		B8 (Uint32 b4_0, Uint32 b4_1) {_ui32[0] = b4_0; _ui32[1] = b4_1;}
		B8 (Uint64 ui64) : _ui64(ui64) {}
		// 反转函数
		operator GE::Uint64() const {return _ui64;}
		// 内部引用函数
		Int8&		I8_0() {return _i8[0];}
		Int8&		I8_1() {return _i8[1];}
		Int8&		I8_2() {return _i8[2];}
		Int8&		I8_3() {return _i8[3];}
		Int8&		I8_4() {return _i8[4];}
		Int8&		I8_5() {return _i8[5];}
		Int8&		I8_6() {return _i8[6];}
		Int8&		I8_7() {return _i8[7];}
		Uint8&		UI8_0() {return _ui8[0];}
		Uint8&		UI8_1() {return _ui8[1];}
		Uint8&		UI8_2() {return _ui8[2];}
		Uint8&		UI8_3() {return _ui8[3];}
		Uint8&		UI8_4() {return _ui8[4];}
		Uint8&		UI8_5() {return _ui8[5];}
		Uint8&		UI8_6() {return _ui8[6];}
		Uint8&		UI8_7() {return _ui8[7];}
		Int16&		I16_0() {return _i16[0];}
		Int16&		I16_1() {return _i16[1];}
		Int16&		I16_2() {return _i16[2];}
		Int16&		I16_3() {return _i16[3];}
		Uint16&		UI16_0() {return _ui16[0];}
		Uint16&		UI16_1() {return _ui16[1];}
		Uint16&		UI16_2() {return _ui16[2];}
		Uint16&		UI16_3() {return _ui16[3];}
		Int32&		I32_0() {return _i32[0];}
		Int32&		I32_1() {return _i32[1];}
		Uint32&		UI32_0() {return _ui32[0];}
		Uint32&		UI32_1() {return _ui32[1];}
		Int64&		I64() {return _i64;}
		Uint64&		UI64() {return _ui64;}
	private:
		Uint64		_ui64;
		Int64		_i64;
		Int8		_i8[8];
		Uint8		_ui8[8];
		Int16		_i16[4];
		Uint16		_ui16[4];
		Int32		_i32[2];
		Uint32		_ui32[2];
	};
}

// 溢出检测
#define UINT64_INC_OVER_FLOW(UI64_0, UI64_1) ((UI64_0&MAX_UINT64) > (MAX_UINT64&~(UI64_1)))
#define UINT64_DEC_OVER_FLOW(UI64_0, UI64_1) ((UI64_0&MAX_UINT64) < (MAX_UINT64&UI64_1))
#define UINT32_INC_OVER_FLOW(UI32_0, UI32_1) ((UI32_0&MAX_UINT32) > (MAX_UINT32&~(UI32_1)))
#define UINT32_DEC_OVER_FLOW(UI32_0, UI32_1) ((UI32_0&MAX_UINT32) < (MAX_UINT32&UI32_1))
#define UINT16_INC_OVER_FLOW(UI16_0, UI16_1) ((UI16_0&MAX_UINT16) > (MAX_UINT16&~(UI16_1)))
#define UINT16_DEC_OVER_FLOW(UI16_0, UI16_1) ((UI16_0&MAX_UINT16) < (MAX_UINT16&UI16_1))
#define UINT8_INC_OVER_FLOW(UI8_0, UI8_1) ((UI8_0&MAX_UINT8) > (MAX_UINT8&~(UI8_1)))
#define UINT8_DEC_OVER_FLOW(UI8_0, UI8_1) ((UI8_0&MAX_UINT8) < (MAX_UINT8&UI8_1))

// 迭代
#define GE_ITER_UI8(_idx, _max) for(GE::Uint8 _idx = 0; _idx < _max; ++_idx)
#define GE_ITER_UI16(_idx, _max) for(GE::Uint16 _idx = 0; _idx < _max; ++_idx)
#define GE_ITER_UI32(_idx, _max) for(GE::Uint32 _idx = 0; _idx < _max; ++_idx)

// 转换
#ifdef WIN
#define GE_AS_B2(v) reinterpret_cast<GE::B2&>(v);GE_ERROR(sizeof(v) == 2);
#define GE_AS_B4(v) reinterpret_cast<GE::B4&>(v);GE_ERROR(sizeof(v) == 4);
#define GE_AS_B8(v) reinterpret_cast<GE::B8&>(v);GE_ERROR(sizeof(v) == 8);
#elif LINUX
#define GE_AS_B2(v) reinterpret_cast<GE::B2&>(v);
#define GE_AS_B4(v) reinterpret_cast<GE::B4&>(v);
#define GE_AS_B8(v) reinterpret_cast<GE::B8&>(v);
#endif

