/************************************************************************
内嵌Python虚拟机
潜规则1：
	所有PyObject*指针或者函数名必须带后缀_NewRef（New Reference）或者
	_BorrowRef（Borrow Reference）来表示其是引用还是借用。
************************************************************************/
#pragma once
#include <string>
#include <vector>
#include "Python.h"
#include "GEInteger.h"

// 如果Py对象对应的c++对象已经被释放了，则设置异常并返回之
#define IF_LOST_C_OBJ_RETURN if(NULL == self->cptr){PyErr_SetString(PyExc_RuntimeError, "C++ obj is NULL!"); return NULL;}

// 参数解析错误
#define PY_PARAM_ERROR(exc)	{PyErr_SetString(PyExc_RuntimeError, exc);return NULL;}

// 返回Python的True或Fasle
#define PY_RETURN_BOOL(b)	if (b) {Py_RETURN_TRUE;} else {Py_RETURN_FALSE;}

// 修复C++传入的NULL指针
#define FIX_NULL_NewRef(pyObj) \
	if(NULL == pyObj) \
	{ \
		Py_INCREF(Py_None); \
		pyObj = Py_None; \
	}

#define FIX_NULL_BorrowRef(pyObj) \
	if(NULL == pyObj) \
	{ \
		pyObj = Py_None; \
	}

// PyNone（引用）
#define Py_None_NewRef Py_None;Py_INCREF(Py_None);

namespace GEPython
{
	/*
	一个用于代理PyObject*的的类，处理了引用计数
	主要用于Python C API的函数返回值
	注意，看清楚Python函数是返回引用还是借用
	*/
	class Object
	{
	public:
		// 构造、析构、赋值函数簇
		Object();
		Object(PyObject* pyObj_NewRef);
		Object(const Object& PO);
		~Object();
		Object& operator=(const Object& PO);
	public:
		void			SetObj_NewRef(PyObject* pyObj_NewRef);					//设置值（引用值）
		void			SetObj_BorrowRef(PyObject* pyObj_BorrowRef);			//设置值（借用）
		void			SetNone();												//设置为None
		bool			IHasExcept();											//当前对象是否发生异常
		bool			PyHasExcept();											//Python是否发生异常
		bool			PyClearExcept();										//清理Python异常
		bool			PyPrintAndClearExcept();								//打印并清理Python异常

		bool			IsNone();												//是否是PyNone
		bool			IsTrue();												//是否是True
		bool			AsTrue();												//是否可以转为True

		PyObject*		GetObj_NewRef();										//获取Object中的PyObject*的引用
		PyObject*		GetObj_BorrowRef();										//获取Object中的PyObject*的借用

	private:
		PyObject*		m_pyObj;												//python对象
		bool			m_bIsNULL;												//接收的参数是否是NULL
	};

	/*
	用于代理PyTuple类，自动处理了引用计数
	*/
	class Tuple
	{
		GE_DISABLE_BOJ_CPY(Tuple);
	public:
		Tuple(GE::Uint16 uSize);
		~Tuple();

	public:
		void			SetObj_NewRef(GE::Uint16 uIdx, PyObject* pyObj_NewRef);				//设置元素（引用）
		void			SetObj_BorrowRef(GE::Uint16 uIdx, PyObject* pyObj_BorrowRef);		//设置元素（借用）
		void			AppendObj_NewRef(PyObject* pyObj_NewRef);							//追加元素（引用）
		void			AppendObj_BorrowRef(PyObject* pyObj_BorrowRef);						//追加元素（借用）

		PyObject*		GetTuple_NewRef();													//获取元组（引用）
		PyObject*		GetTuple_BorrowRef();												//获取元组（借用）

	private:
		GE::Uint16		m_uIdx;
		GE::Uint16		m_uTupleSize;
		PyObject*		m_pyTuple;
	};

	/*
	用于代理PyList类，自动处理了引用计数
	*/
	class List
	{
		GE_DISABLE_BOJ_CPY(List);
	public:
		List();
		~List();

	public:
		void			AppendObj_NewRef(PyObject* pyObj_NewRef);			//追加元素（引用）
		void			AppendObj_BorrowRef(PyObject* pyObj_BorrowRef);		//追加元素（借用）

		PyObject*		GetList_NewRef();									//获取列表（引用）
		PyObject*		GetList_BorrowRef();								//获取列表（借用）

	private:
		PyObject*		m_pyList;
	};

	/*
	用于代理PySet类，自动处理了引用计数
	*/
	class Set
	{
		GE_DISABLE_BOJ_CPY(Set);
	public:
		Set();
		~Set();
	public:
		void			AddObj_NewRef(PyObject* pyObj_NewRef);				//增加元素（引用）
		void			AddObj_BorrowRef(PyObject* pyObj_BorrowRef);		//增加元素（借用）

		PyObject*		GetSet_NewRef();									//获取集合（引用）
		PyObject*		GetSet_BorrowRef();									//获取集合（借用）
	private:
		PyObject*		m_pySet;
	};

	/*
	用于代理PyDict类，自动处理了引用计数
	*/
	class Dict
	{
		GE_DISABLE_BOJ_CPY(Dict);
	public:
		Dict();
		~Dict();
	public:
		void			SetObj_NewRef(PyObject* pyKey_NewRef, PyObject* pyValue_NewRef);			//设置元素（引用）
		void			SetObj_BorrowRef(PyObject* pyKey_BorrowRef, PyObject* pyValue_BorrowRef);	//设置元素（借用）

		PyObject*		GetDict_NewRef();															//获取字典（引用）
		PyObject*		GetDict_BorrowRef();														//获取字典（借用）
	private:
		PyObject*		m_pyDict;
	};

	/*
	用于代理PyFunction类，自动处理了引用计数
	*/
	class Function
	{
		GE_DISABLE_BOJ_CPY(Function);
	public:
		Function();
		Function(const char* pModule, const char* pFunction);
		~Function();

		bool			IsNone();													//是否是PyNone
	public:
		void			Load(const char* pModule, const char* pFunction);			//载入函数
		void			Load(PyObject* pyFun_BorrowRef);							//载入函数
		void			Call();														//调用函数
		void			Call(const char* sFormat, ... );							//调用函数
		void			Call(PyObject* pyArgs_BorrowRef);							//调用函数
		void			CallObjArgs(GE::Uint16 uCount, ...);						//调用函数
		Object			CallAndResult(const char* sFormat, ... );					//调用函数并返回结果
		PyObject*		GetFun_NewRef();
		PyObject*		GetFun_BorrowRef();
	private:
		Object			m_pyFunction;
		std::string		m_sModule;
		std::string		m_sFunction;
	};

	// 初始化python虚拟机
	void				InitPython();
	// 结束python虚拟机
	void				FinalPython();
	// 插入python搜寻路径
	void				InsertPythonPath(GE::Uint32 uPos, const char* sPath);
	// 获取某个模块的某个属性
	Object				GetModuleAttr(const char* sModuleName, const char* sAttrName);
	long				GetModuleLong(const char* sModuleName, const char* sAttrName);
	bool				GetModuleBool(const char* sModuleName, const char* sAttrName);
	void				GetModuleString(const char* sModuleName, const char* sAttrName, std::string& Str);
	// 调用某个模块的某个函数
	void				CallModulFunction(const char* sModuleName, const char* sFunctionName, const char* sFormat, ... );
	// 初始化引擎的Python模块
	void				PyDateTime_Init(void);
	void				PyProcess_Init(void);
	void				PyNetEnv_Init(void);
	void				PyNetMessage_Init(void);
	// python整数和GameEngine整数之间的转换
	PyObject*			PyObjFromI32(GE::Int32 i32);
	PyObject*			PyObjFromUI32(GE::Uint32 ui32);
	PyObject*			PyObjFromI64(GE::Int64 i64);
	PyObject*			PyObjFromUI64(GE::Uint64 ui64);
	PyObject*			PyObjFromLong(long l);
	// 注意这下面的函数可能会引起Python异常，要注意处理啊
	GE::Int32			PyObjAsI32(PyObject* pyObj);
	GE::Uint32			PyObjAsUI32(PyObject* pyObj);
	GE::Int64			PyObjAsI64(PyObject* pyObj);
	GE::Uint64			PyObjAsUI64(PyObject* pyObj);
	long				PyObjAsLong(PyObject* pyObj);
	// 注意这下面的函数在内部处理了异常
	bool				PyObjToUI8(PyObject* pyObj, GE::Uint8& i8);
	bool				PyObjToUI16(PyObject* pyObj, GE::Uint16& i16);
	bool				PyObjToUI32(PyObject* pyObj, GE::Uint32& ui32);
	bool				PyObjToUI64(PyObject* pyObj, GE::Uint64& ui64);

	bool				PyObjToI32(PyObject* pyObj, GE::Int32& i32);
	bool				PyObjToI64(PyObject* pyObj, GE::Int64& i64);
	bool				PyObjToLong(PyObject* pyObj, long& l);
	bool				PyObjToStr(PyObject* pyObj, char** pz);
	bool				PyObjToStrAndSize(PyObject* pyObj, char** pz, GE::Uint16& uSize);
	// 时间日期（必须归结到这里）
	PyObject*			PyObjFromDatetime(GE::Uint32 y, GE::Uint32 m, GE::Uint32 d, GE::Uint32 H, GE::Uint32 M, GE::Uint32 S);
	bool				PyDateTimeCheck(PyObject* pyObj);
}

