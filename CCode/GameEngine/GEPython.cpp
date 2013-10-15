/*我是UTF8无签名编码*/
#include <sstream>
#include "GEPython.h"
#include "datetime.h"
#include "GEIO.h"

// 如果Python函数发生了异常，则打印之并做
#define IF_PYEXCEPT_DO(cobject, dosomething) if(cobject.PyPrintAndClearExcept()){dosomething;}

// 检测并清理Python异常，返回之
#define CHECK_CLEAR_PYEXECEPT_RETURN if(PyErr_Occurred()) {PyErr_Clear();return false;} else {return true;}

namespace GEPython
{
	Object::Object()
		: m_pyObj(NULL)
		, m_bIsNULL(false)
	{
		// 默认为PyNone
		m_pyObj = Py_None_NewRef;
	}

	Object::Object(PyObject* pyObj_NewRef)
		: m_pyObj(NULL)
	{
		// 修正NULL
		if (NULL == pyObj_NewRef)
		{
			this->m_bIsNULL = true;
			this->m_pyObj = Py_None_NewRef;
		}
		else
		{
			this->m_bIsNULL = false;
			// 注意这里参数pyObj_NewRef是引用了，故直接赋值
			this->m_pyObj = pyObj_NewRef;
		}
	}

	Object::Object( const Object& PO )
	{
		// 拷贝构造函数（PO.m_GetObj_NewRef不会为NULL）
		this->m_pyObj = PO.m_pyObj;
		this->m_bIsNULL = PO.m_bIsNULL;
		// 注意，这里拷贝了一次，需要增加引用计数
		Py_INCREF(m_pyObj);
	}

	Object::~Object()
	{
		// 析构的时候，减少引用计数
		Py_DECREF(m_pyObj);
		// 将C++对象和Python对象解绑（防止崩溃，这里用了个小技巧）
		this->m_pyObj = Py_None;
	}

	Object& Object::operator=( const Object& PO )
	{
		// 注意，这里不同于构造函数，先要尝试减少前一个m_pyObj的引用计数
		Py_DECREF(m_pyObj);
		// 赋值（PO.m_GetObj_NewRef不会为NULL）
		this->m_pyObj = PO.m_pyObj;
		this->m_bIsNULL = PO.m_bIsNULL;
		// 注意，这里拷贝了一次，需要增加引用计数
		Py_INCREF(m_pyObj);
		// 返回自己
		return *this;
	}

	void Object::SetObj_NewRef(PyObject* pyObj_NewRef)
	{
		// 修正NULL
		if (NULL == pyObj_NewRef)
		{
			this->m_bIsNULL = true;
			pyObj_NewRef = Py_None_NewRef;
		}
		else
		{
			this->m_bIsNULL = false;
		}
		// 注意，这里不同于构造函数，先要尝试减少前一个m_pyObj的引用计数
		Py_DECREF(m_pyObj);
		// 注意，这里参数pyObj_NewRef是引用，不需要增加计数了
		this->m_pyObj = pyObj_NewRef;
	}

	void Object::SetObj_BorrowRef(PyObject* pyObj_BorrowRef)
	{
		// 修正NULL
		if (NULL == pyObj_BorrowRef)
		{
			this->m_bIsNULL = true;
			pyObj_BorrowRef = Py_None;
		}
		else
		{
			this->m_bIsNULL = false;
		}
		// 注意，这里参数pyObj_BorrowRef是借用，还有可能等于m_pyObj并且为1，故要先将pyObj_BorrowRef变为引用
		// 防止减少m_pyObj的引用计数时，对象被摧毁
		Py_INCREF(pyObj_BorrowRef);
		// 注意，这里不同于构造函数，先要尝试减少前一个m_pyObj的引用计数
		Py_DECREF(m_pyObj);
		// 注意，这里参数pyObj_BorrowRef是已经变成引用了，不需要增加计数了
		this->m_pyObj = pyObj_BorrowRef;
	}

	void Object::SetNone()
	{
		// 注意，这里不同于构造函数，先要尝试减少前一个PyObject的引用计数
		Py_DECREF(m_pyObj);
		// 赋值为None并增加引用计数
		this->m_pyObj = Py_None_NewRef;
	}

	bool Object::IHasExcept()
	{
		return this->m_bIsNULL;
	}

	bool Object::PyHasExcept()
	{
		/*
		Return value: Borrowed reference.If not set, return NULL.
		*/
		return NULL != PyErr_Occurred();
	}

	bool Object::PyClearExcept()
	{
		if (this->PyHasExcept())
		{
			PyErr_Clear();
			return true;
		}
		else
		{
			return false;
		}
	}

	bool Object::PyPrintAndClearExcept()
	{
		if (this->PyHasExcept())
		{
			PyErr_Print();
			return true;
		}
		else
		{
			return false;
		}
	}

	bool Object::IsNone()
	{
		return Py_None == m_pyObj;
	}

	bool Object::IsTrue()
	{
		return Py_True == m_pyObj;
	}

	bool Object::AsTrue()
	{
		/*
		Returns 1 if the object o is considered to be true, and 0 otherwise.
		This is equivalent to the Python expression not not o. On failure, return -1.
		*/
		return 1 == PyObject_IsTrue(this->m_pyObj);
	}

	PyObject* Object::GetObj_NewRef()
	{
		Py_INCREF(m_pyObj);
		return this->m_pyObj;
	}

	PyObject* Object::GetObj_BorrowRef()
	{
		return this->m_pyObj;
	}


	Tuple::Tuple( GE::Uint16 uSize )
		: m_uIdx(0)
		, m_uTupleSize(uSize)
	{
		/*
		Return value: New reference.
		Return a new tuple object of size len, or NULL on failure.
		*/
		m_pyTuple = PyTuple_New(uSize);
	}

	Tuple::~Tuple()
	{
		// 释放掉Tuple的引用
		Py_DECREF(m_pyTuple);
		// 将C++对象和Python对象解绑
		this->m_pyTuple = Py_None;
	}

	void Tuple::SetObj_NewRef( GE::Uint16 uIdx, PyObject* pyObj_NewRef )
	{
		// 修正NULL
		FIX_NULL_NewRef(pyObj_NewRef);
		if (uIdx < m_uTupleSize)
		{
			/*
			Like PyTuple_SetItem(), but does no error checking, and should only be used to fill in brand new tuples.
			Note This function "steals" a reference to o.
			*/
			PyTuple_SET_ITEM(m_pyTuple, uIdx, pyObj_NewRef);
		}
		else
		{
			/*
			注意，这里设置失败，要把偷用来的计数-1
			*/
			Py_DECREF(pyObj_NewRef);
			GE_EXC<<"tuple index out of range."<<GE_END;
		}
	}

	void Tuple::SetObj_BorrowRef( GE::Uint16 uIdx, PyObject* pyObj_BorrowRef )
	{
		// 修正NULL
		FIX_NULL_BorrowRef(pyObj_BorrowRef);
		/*
		注意，这里是偷用，要先将引用计数+1
		*/
		Py_INCREF(pyObj_BorrowRef);
		this->SetObj_NewRef(uIdx, pyObj_BorrowRef);
	}

	void Tuple::AppendObj_NewRef( PyObject* pyObj_NewRef )
	{
		this->SetObj_NewRef(m_uIdx, pyObj_NewRef);
		++m_uIdx;
	}

	void Tuple::AppendObj_BorrowRef( PyObject* pyObj_BorrowRef )
	{
		this->SetObj_BorrowRef(m_uIdx, pyObj_BorrowRef);
		++m_uIdx;
	}

	PyObject* Tuple::GetTuple_NewRef()
	{
		// 完全设置了
		if (m_uIdx == m_uTupleSize)
		{
			Py_INCREF(m_pyTuple);
			return this->m_pyTuple;
		}
		else
		{
			GE_EXC<<"tuple size("<<m_uTupleSize<<") != uIdx("<<m_uIdx<<")."<<GE_END;
			Py_RETURN_NONE;
		}
	}

	PyObject* Tuple::GetTuple_BorrowRef()
	{
		if (m_uIdx == m_uTupleSize)
		{
			return this->m_pyTuple;
		}
		else
		{
			GE_EXC<<"tuple size("<<m_uTupleSize<<") != uIdx("<<m_uIdx<<")."<<GE_END;
			Py_RETURN_NONE;
		}
	}


	List::List()
	{
		/*
		Return value: New reference.
		Return a new list of length len on success, or NULL on failure.
		If len is greater than zero, the returned list object’s items are set to NULL.
		Thus you cannot use abstract API functions such as PySequence_SetItem() or expose the object to Python code before setting all items to a real object with PyList_SetItem().
		*/
		this->m_pyList = PyList_New(0);
	}

	List::~List()
	{
		Py_DECREF(m_pyList);
		// 将C++对象和Python对象解绑
		this->m_pyList = Py_None;
	}

	void List::AppendObj_NewRef( PyObject* pyObj_NewRef )
	{
		// 修正NULL
		FIX_NULL_NewRef(pyObj_NewRef);
		/*
		这里一定要先AppendObj_BorrowRef，再将引用计数-1
		因为引用计数有可能就是1
		*/
		this->AppendObj_BorrowRef(pyObj_NewRef);
		Py_DECREF(pyObj_NewRef);
	}

	void List::AppendObj_BorrowRef( PyObject* pyObj_BorrowRef )
	{
		// 修正NULL
		FIX_NULL_NewRef(pyObj_BorrowRef);
		/*
		Append the object item at the end of list list. Return 0 if successful; return -1 and set an exception if unsuccessful.
		*/
		if (-1 == PyList_Append(this->m_pyList, pyObj_BorrowRef))
		{
			PyErr_Print();
		}
	}

	PyObject* List::GetList_NewRef()
	{
		Py_INCREF(m_pyList);
		return this->m_pyList;
	}

	PyObject* List::GetList_BorrowRef()
	{
		return this->m_pyList;
	}


	Set::Set()
	{
		/*
		Return value: New reference.
		Return a new set containing objects returned by the iterable.
		The iterable may be NULL to create a new empty set.
		Return the new set on success or NULL on failure.
		Raise TypeError if iterable is not actually iterable. The constructor is also useful for copying a set (c=set(s)).
		*/
		this->m_pySet = PySet_New(NULL);
	}

	Set::~Set()
	{
		Py_DECREF(m_pySet);
		// 将C++对象和Python对象解绑
		this->m_pySet = Py_None;
	}

	void Set::AddObj_NewRef( PyObject* pyObj_NewRef )
	{
		// 修复NULL
		FIX_NULL_NewRef(pyObj_NewRef);
		/*
		这里一定要先AddObj_BorrowRef，再将引用计数-1
		因为引用计数有可能就是1
		*/
		this->AddObj_BorrowRef(pyObj_NewRef);
		Py_DECREF(pyObj_NewRef);
	}

	void Set::AddObj_BorrowRef( PyObject* pyObj_BorrowRef )
	{
		// 修复NULL
		FIX_NULL_BorrowRef(pyObj_BorrowRef);
		/*
		Add key to a set instance. Does not apply to frozenset instances. Return 0 on success or -1 on failure.
		*/
		if (-1 == PySet_Add(m_pySet, pyObj_BorrowRef))
		{
			PyErr_Print();
		}
	}

	PyObject* Set::GetSet_NewRef()
	{
		Py_INCREF(m_pySet);
		return m_pySet;
	}

	PyObject* Set::GetSet_BorrowRef()
	{
		return m_pySet;
	}


	Dict::Dict()
	{
		/*
		Return value: New reference.
		Return a new empty dictionary, or NULL on failure.
		*/
		this->m_pyDict = PyDict_New();
	}

	Dict::~Dict()
	{
		Py_DECREF(m_pyDict);
		// 将C++对象和Python对象解绑
		this->m_pyDict = Py_None;
	}

	void Dict::SetObj_NewRef( PyObject* pyKey_NewRef, PyObject* pyValue_NewRef )
	{
		// 修正NULL
		FIX_NULL_NewRef(pyKey_NewRef);
		FIX_NULL_NewRef(pyValue_NewRef);
		/*
		这里一定要先SetObj_BorrowRef，再将引用计数-1
		因为引用计数有可能就是1
		*/
		this->SetObj_BorrowRef(pyKey_NewRef, pyValue_NewRef);
		Py_DECREF(pyKey_NewRef);
		Py_DECREF(pyValue_NewRef);
	}

	void Dict::SetObj_BorrowRef( PyObject* pyKey_BorrowRef, PyObject* pyValue_BorrowRef )
	{
		// 修正NULL
		FIX_NULL_BorrowRef(pyKey_BorrowRef);
		FIX_NULL_BorrowRef(pyValue_BorrowRef);
		/*
		Insert value into the dictionary p with a key of key. key must be hashable;
		if it isn’t, TypeError will be raised. Return 0 on success or -1 on failure.
		*/
		if (-1 == PyDict_SetItem(m_pyDict, pyKey_BorrowRef, pyValue_BorrowRef))
		{
			PyErr_Print();
		}
	}

	PyObject* Dict::GetDict_NewRef()
	{
		Py_INCREF(m_pyDict);
		return m_pyDict;
	}

	PyObject* Dict::GetDict_BorrowRef()
	{
		return m_pyDict;
	}


	Function::Function()
	{

	}

	Function::Function( const char* pModule, const char* pFunction )
	{
		this->Load(pModule, pFunction);
	}

	Function::~Function()
	{

	}

	void Function::Load( const char* pModule, const char* pFunction )
	{
#ifdef WIN
		GE_ERROR(m_pyFunction.IsNone());
#endif
		m_pyFunction = GetModuleAttr(pModule, pFunction);
		this->m_sModule = pModule;
		this->m_sFunction = pFunction;
	}


	void Function::Load( PyObject* pyFun_BorrowRef )
	{
#ifdef WIN
		GE_ERROR(m_pyFunction.IsNone());
#endif
		m_pyFunction.SetObj_BorrowRef(pyFun_BorrowRef);
	}

	void Function::Call()
	{
		Tuple tup(0);
		this->Call(tup.GetTuple_BorrowRef());
	}

	void Function::Call( const char* sFormat, ... )
	{
		PyObject* pyArg_NewRef = NULL;
		va_list va;
		va_start( va, sFormat );
		if (sFormat && *sFormat)
		{
			/*
			Return value: New reference.
			Create a new value based on a format string similar to those accepted by the PyArg_Parse*() family of functions and a sequence of values.
			Returns the value or NULL in the case of an error; an exception will be raised if NULL is returned.
			*/
			pyArg_NewRef = Py_VaBuildValue(sFormat, va);
		}
		else
		{
			/*
			Return value: New reference.
			Return a new tuple object of size len, or NULL on failure.
			*/
			pyArg_NewRef = PyTuple_New(0);
		}
		va_end( va );
		if (NULL == pyArg_NewRef)
		{
			GE_EXC<<"("<<this->m_sModule<<" "<<this->m_sFunction<<") param error."<<GE_END;
		}
		else
		{
			this->Call(pyArg_NewRef);
			Py_DECREF(pyArg_NewRef);
		}
	}

	Object Function::CallAndResult( const char* sFormat, ... )
	{
		Object Arg;
		va_list va;
		va_start( va, sFormat );
		if (sFormat && *sFormat)
		{
			/*
			Return value: New reference.
			Create a new value based on a format string similar to those accepted by the PyArg_Parse*() family of functions and a sequence of values.
			Returns the value or NULL in the case of an error; an exception will be raised if NULL is returned.
			*/
			Arg.SetObj_NewRef(Py_VaBuildValue(sFormat, va));
		}
		else
		{
			/*
			Return value: New reference.
			Return a new tuple object of size len, or NULL on failure.
			*/
			Arg.SetObj_NewRef(PyTuple_New(0));
		}
		va_end( va );
		if (Arg.IHasExcept())
		{
			GE_EXC<<"("<<this->m_sModule<<" "<<this->m_sFunction<<") param error."<<GE_END;
			Arg.PyPrintAndClearExcept();
			return NULL;
		}
		/*
		Return value: New reference.
		Call a callable Python object callable_object, with arguments given by the tuple args.
		If no arguments are needed, then args may be NULL. Returns the result of the call on success, or NULL on failure.
		*/
		Object Result = PyObject_CallObject(m_pyFunction.GetObj_BorrowRef(), Arg.GetObj_BorrowRef());
		if (Result.IHasExcept())
		{
			GE_EXC<<"("<<this->m_sModule<<" "<<this->m_sFunction<<") result error."<<GE_END;
			Result.PyPrintAndClearExcept();
			return NULL;
		}
		return Result;
	}

	void Function::Call( PyObject* pyArgs_BorrowRef )
	{
		/*
		Return value: New reference.
		Call a callable Python object callable_object, with arguments given by the tuple args.
		If no arguments are needed, then args may be NULL. Returns the result of the call on success, or NULL on failure.
		*/
		PyObject* pyResult_NewRef = PyObject_CallObject(m_pyFunction.GetObj_BorrowRef(), pyArgs_BorrowRef);
		// 如果有异常，打印之
		if (NULL == pyResult_NewRef)
		{
			PyErr_Print();
		}
		// 否则是否结果引用
		else
		{
			Py_DECREF(pyResult_NewRef);
		}
	}

	void Function::CallObjArgs( GE::Uint16 uCount, ... )
	{
		// 修正参数个数
		if (uCount == 0)
		{
			va_list vac;
			va_start(vac, uCount);
			while (((PyObject *)va_arg(vac, PyObject *)) != NULL)
			{
				++uCount;
			}
			va_end(vac);
		}
		// 构建参数
		Tuple pyArgs(uCount);
		va_list vag;
		va_start(vag, uCount);
		GE_ITER_UI16(idx, uCount)
		{
			PyObject* pyObj_BorrowRef = (PyObject *)va_arg(vag, PyObject*);
			pyArgs.AppendObj_BorrowRef(pyObj_BorrowRef);
		}
		PyObject* pyObj_BorrowRef = (PyObject *)va_arg(vag, PyObject*);
		/*
		注意下，这里是检测函数栈中的参数是否以NULL结尾，故NULL != pyObj_BorrowRef表示uCount与压栈参数个数不匹配。
		*/
		if (NULL != pyObj_BorrowRef)
		{
			GE_EXC<<"("<<this->m_sModule<<" "<<this->m_sFunction<<") param error."<<GE_END;
		}
		va_end(vag);
		
		return this->Call(pyArgs.GetTuple_BorrowRef());
	}

	PyObject* Function::GetFun_NewRef()
	{
		return this->m_pyFunction.GetObj_BorrowRef();
	}

	PyObject* Function::GetFun_BorrowRef()
	{
		return this->m_pyFunction.GetObj_NewRef();
	}

	bool Function::IsNone()
	{
		return this->m_pyFunction.IsNone();
	}

	void InitPython()
	{
		if (Py_IsInitialized())
		{
			return;
		}
		Py_Initialize();
		// 修正Python的路径
		PyRun_SimpleString("import sys;sys.path = []");
		InsertPythonPath(0, "../PyLib/site-packages/");
#ifdef WIN
		InsertPythonPath(0, "../PyLib/Python27.zip");
		InsertPythonPath(0, "../PyLib/DLLs/");
#elif LINUX
		InsertPythonPath(0, "../PyLib/lib/python2.7");
		InsertPythonPath(0, "../PyLib/lib/python2.7.zip");
		InsertPythonPath(0, "../PyLib/lib/python2.7/plat-linux2");
		InsertPythonPath(0, "../PyLib/lib/python2.7/lib-tk");
		InsertPythonPath(0, "../PyLib/lib/python2.7/lib-old");
		InsertPythonPath(0, "../PyLib/lib/python2.7/lib-dynload");
#endif
		InsertPythonPath(0, "../PyCode/");
		// 因为使用了datetime模块，故要做这个初始化
		PyDateTime_IMPORT;
		// 初始化引擎模块
		GEPython::PyProcess_Init();
		GEPython::PyDateTime_Init();
		GEPython::PyNetEnv_Init();
		GEPython::PyNetMessage_Init();
	}

	void FinalPython()
	{
		Py_Finalize();
	}

	void InsertPythonPath( GE::Uint32 uPos, const char* sPath )
	{
		std::stringstream ss;
		ss<<"import sys\nif '"<<sPath<<"' not in sys.path: sys.path.insert("<<uPos<<", '"<<sPath<<"')";
		PyRun_SimpleString(ss.str().c_str());
	}

	Object GetModuleAttr( const char* sModuleName, const char* sAttrName )
	{
		// 根据模块名，查找模块
		Object spModule = PyImport_ImportModule(sModuleName);
		IF_PYEXCEPT_DO(spModule, return NULL);
		// 根据函数名，查找函数
		Object spAttr = PyObject_GetAttrString(spModule.GetObj_BorrowRef(), sAttrName);
		IF_PYEXCEPT_DO(spAttr, return NULL);
		// 返回结果
		return spAttr;
	}

	long GetModuleLong( const char* sModuleName, const char* sAttrName )
	{
		Object pyl = GetModuleAttr(sModuleName, sAttrName);
		long l = 0;
		PyObjToLong(pyl.GetObj_BorrowRef(), l);
		return l;
	}

	bool GetModuleBool( const char* sModuleName, const char* sAttrName )
	{
		Object pyl = GetModuleAttr(sModuleName, sAttrName);
		return pyl.AsTrue();
	}

	void GetModuleString( const char* sModuleName, const char* sAttrName, std::string& Str )
	{
		Object pys = GetModuleAttr(sModuleName, sAttrName);
		char* p = NULL;
		if (PyObjToStr(pys.GetObj_BorrowRef(), &p))
		{
			Str.append(p);
		}
	}

	void CallModulFunction( const char* sModuleName, const char* sFunctionName, const char* sFormat, ... )
	{
		// 根据模块名，查找模块
		Object spModule = PyImport_ImportModule(sModuleName);
		if (spModule.IHasExcept())
		{
			spModule.PyPrintAndClearExcept();
			return;
		}
		// 根据函数名，查找函数
		Object spFunction = PyObject_GetAttrString(spModule.GetObj_BorrowRef(), sFunctionName);
		if (spFunction.IHasExcept())
		{
			spFunction.PyPrintAndClearExcept();
			return;
		}
		// 根据c参数列表，构建py函数参数
		Object spArgs;
		va_list va;
		va_start( va, sFormat );
		if (sFormat && *sFormat)
		{
			/*
			Return value: New reference.
			Create a new value based on a format string similar to those accepted by the PyArg_Parse*() family of functions and a sequence of values.
			Returns the value or NULL in the case of an error; an exception will be raised if NULL is returned.
			*/
			spArgs.SetObj_NewRef(Py_VaBuildValue(sFormat, va));
		}
		else
		{
			/*
			Return value: New reference.
			Return a new tuple object of size len, or NULL on failure.
			*/
			spArgs.SetObj_NewRef(PyTuple_New(0));
		}
		va_end( va );
		if (spArgs.IHasExcept())
		{
			spArgs.PyPrintAndClearExcept();
			return;
		}
		/*
		Return value: New reference.
		Call a callable Python object callable_object, with arguments given by the tuple args.
		If no arguments are needed, then args may be NULL. Returns the result of the call on success, or NULL on failure.
		*/
		Object spResult = PyObject_CallObject(spFunction.GetObj_BorrowRef(), spArgs.GetObj_BorrowRef());
		if (spResult.IHasExcept())
		{
			spResult.PyPrintAndClearExcept();
		}
	}

	PyObject* PyObjFromI32( GE::Int32 i32 )
	{
		return PyInt_FromLong(i32);
	}

	PyObject* PyObjFromUI32( GE::Uint32 ui32 )
	{
		return PyInt_FromSize_t(ui32);
	}

	PyObject* PyObjFromI64( GE::Int64 i64 )
	{
		return PyLong_FromLongLong(i64);
	}

	PyObject* PyObjFromUI64( GE::Uint64 ui64 )
	{
		return PyLong_FromUnsignedLongLong(ui64);
	}

	PyObject* PyObjFromLong( long l )
	{
		return PyInt_FromLong(l);
	}

	GE::Int32 PyObjAsI32( PyObject* pyObj )
	{
		return PyInt_AsLong(pyObj);
	}

	GE::Uint32 PyObjAsUI32( PyObject* pyObj )
	{
		return PyInt_AsUnsignedLongMask(pyObj);
	}

	GE::Int64 PyObjAsI64( PyObject* pyObj )
	{
		return PyLong_AsLongLong(pyObj);
	}

	GE::Uint64 PyObjAsUI64( PyObject* pyObj )
	{
		return PyInt_AsUnsignedLongLongMask(pyObj);
	}

	long PyObjAsLong( PyObject* pyObj )
	{
		return PyInt_AsLong(pyObj);
	}

	bool PyObjToUI8( PyObject* pyObj, GE::Uint8& i8 )
	{
		i8 = static_cast<GE::Uint8>(PyObjAsUI64(pyObj));
		CHECK_CLEAR_PYEXECEPT_RETURN;
	}

	bool PyObjToUI16( PyObject* pyObj, GE::Uint16& i16 )
	{
		i16 = static_cast<GE::Uint16>(PyObjAsUI64(pyObj));
		CHECK_CLEAR_PYEXECEPT_RETURN;
	}

	bool PyObjToUI32( PyObject* pyObj, GE::Uint32& ui32 )
	{
		ui32 = static_cast<GE::Uint32>(PyObjAsUI64(pyObj));
		CHECK_CLEAR_PYEXECEPT_RETURN;
	}

	bool PyObjToUI64( PyObject* pyObj, GE::Uint64& ui64 )
	{
		ui64 = PyObjAsUI64(pyObj);
		CHECK_CLEAR_PYEXECEPT_RETURN;
	}

	bool PyObjToI64( PyObject* pyObj, GE::Int64& i64 )
	{
		i64 = PyObjAsI64(pyObj);
		CHECK_CLEAR_PYEXECEPT_RETURN;
	}

	bool PyObjToI32(PyObject* pyObj, GE::Int32& i32)
	{
		i32 = PyObjAsI32(pyObj);
		CHECK_CLEAR_PYEXECEPT_RETURN;
	}

	bool PyObjToLong( PyObject* pyObj, long& l )
	{
		l = PyObjAsLong(pyObj);
		CHECK_CLEAR_PYEXECEPT_RETURN;
	}

	bool PyObjToStr( PyObject* pyObj, char** pz )
	{
		if (PyString_CheckExact(pyObj))
		{
			*pz = PyString_AS_STRING(pyObj);
			return true;
		}
		else
		{
			return false;
		}
	}

	bool PyObjToStrAndSize( PyObject* pyObj, char** pz, GE::Uint16& uSize )
	{
		if (PyString_CheckExact(pyObj))
		{
			*pz = PyString_AS_STRING(pyObj);
			uSize = static_cast<GE::Uint16>(PyString_GET_SIZE(pyObj));
			return true;
		}
		else
		{
			return false;
		}
	}

	PyObject* PyObjFromDatetime( GE::Uint32 y, GE::Uint32 m, GE::Uint32 d, GE::Uint32 H, GE::Uint32 M, GE::Uint32 S )
	{
		return (PyObject*)PyDateTime_FromDateAndTime(static_cast<int>(y), static_cast<int>(m), static_cast<int>(d),
			static_cast<int>(H), static_cast<int>(M), static_cast<int>(S), 0);
	}

	bool PyDateTimeCheck( PyObject* pyObj )
	{
		return PyDateTime_Check(pyObj);
	}
}

