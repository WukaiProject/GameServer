/*我是UTF8无签名编码*/
#include "GETick.h"
#include "GEDateTime.h"
#include "GEProcess.h"

// Tick每秒可触发的个数，多余的会移到下一秒触发
#define MAX_CALLBACK_PER_DRIVER			5000
// 小Tick可持续运行的秒数（2^31 / 1000）
#define SMALL_TICK_LOW					1000
#define SMALL_TICK_SPE					2147483

//////////////////////////////////////////////////////////////////////////
// GETick
//////////////////////////////////////////////////////////////////////////
GETick::GETick(void)
{
	// 累加ID要附带进程ID，生成全球唯一ID，这样才可以持久化
	this->m_uLowerID.UI16_1() = GEProcess::Instance()->ProcessID();
}

GETick::GETick( PyObject* pPyOwner_BorrowRef )
{
	// 注意，这里传进来的参数是借用，需要保存则要增加引用计数
	this->m_pyOwner.SetObj_BorrowRef(pPyOwner_BorrowRef);
	// 累加ID要附带进程ID，生成全球唯一ID，这样才可以持久化
	this->m_uLowerID.UI16_1() = GEProcess::Instance()->ProcessID();
}

GETick::~GETick(void)
{
	// 这里释放还没有触发的Tick
	TickReactor::iterator _iter = this->m_TickReactorMap.begin();
	while(_iter != this->m_TickReactorMap.end())
	{
		Py_DECREF(_iter->second.m_callback);
		Py_DECREF(_iter->second.m_param);
		this->m_TickReactorMap.erase(_iter++);
	}
	this->m_TickReactorMap.clear();
}

GE::Int64 GETick::RegTick( GE::Uint32 uTimeSec, PyObject* pyCallBack_BorrowRef, PyObject* pyParam_BorrowRef )
{
	// 累加ID的低32位
	++m_uLowerID.UI32();
	// 计算超时时间
	GE::Uint32 uTouchTime = GEDateTime::Instance()->Seconds() + uTimeSec;
	// 构建ID，超时时间为高32位，低32位为累进计数，使得存储到Map中是按时间排序的
	GE::B8 uID(m_uLowerID.UI32(), uTouchTime);
	// 增加Python的引用计数
	Py_INCREF(pyCallBack_BorrowRef);
	Py_INCREF(pyParam_BorrowRef);
	// 存入Map
	_PyCallBack PCB(pyCallBack_BorrowRef, pyParam_BorrowRef);
	this->m_TickReactorMap.insert(std::make_pair(uID.I64(), PCB));
	// 返回一个唯一ID
	return uID.I64();
}

bool GETick::UnregTick( GE::Int64 uID )
{
	TickReactor::iterator _iter = this->m_TickReactorMap.find(uID);
	// 根据ID查找回调结构体
	if (_iter == this->m_TickReactorMap.end())
	{
		return false;
	}
	// 减少引用计数
	Py_DECREF(_iter->second.m_callback);
	Py_DECREF(_iter->second.m_param);
	this->m_TickReactorMap.erase(_iter);
	return true;
}

bool GETick::TriggerTick( GE::Int64 uID )
{
	return this->TriggerTick(uID, Py_None);
}

bool GETick::TriggerTick( GE::Int64 uID, PyObject* pyTrigger_BorrowRef )
{
	TickReactor::iterator _iter = this->m_TickReactorMap.find(uID);
	// 根据ID查找回调结构体
	if (_iter == this->m_TickReactorMap.end())
	{
		return false;
	}
	// 注意这里要拷贝一份再从Map中删除
	_PyCallBack PC = _iter->second;
	// 先从Map中删除,防止在下面的函数调用中修改_iter
	this->m_TickReactorMap.erase(_iter);
	// 触发
	if (this->m_pyOwner.IsNone())
	{
		/*
		Return value: New reference.
		Call a callable Python object callable, with a variable number of PyObject* arguments. The arguments are provided as a variable number of parameters followed by NULL.
		Returns the result of the call on success, or NULL on failure.
		*/
		PyObject* pyResult_NewRef = PyObject_CallFunctionObjArgs(PC.m_callback, pyTrigger_BorrowRef, PC.m_param, NULL);
		if (NULL == pyResult_NewRef)
		{
			PyErr_Print();
		}
		else
		{
			Py_DECREF(pyResult_NewRef);
		}
	}
	else
	{
		/*
		Return value: New reference.
		Call a callable Python object callable, with a variable number of PyObject* arguments. The arguments are provided as a variable number of parameters followed by NULL.
		Returns the result of the call on success, or NULL on failure.
		*/
		PyObject* pyResult_NewRef = PyObject_CallFunctionObjArgs(PC.m_callback, this->m_pyOwner.GetObj_BorrowRef(), pyTrigger_BorrowRef, PC.m_param, NULL);
		if (NULL == pyResult_NewRef)
		{
			PyErr_Print();
		}
		else
		{
			Py_DECREF(pyResult_NewRef);
		}
	}
	// 减少引用计数
	Py_DECREF(PC.m_callback);
	Py_DECREF(PC.m_param);
	return true;
}

void GETick::CallPerSecond()
{
	// 没有Tick，直接返回之
	if (0 == this->m_TickReactorMap.size())
	{
		return;
	}
	// 一个全局静态的回调结构体数组的缓存
	static _PyCallBack m_callbackArray[MAX_CALLBACK_PER_DRIVER];
	// 计数现在的超时时间
	GE::B8 _now(0, GEDateTime::Instance()->Seconds() + 1);
	// 查找已经超时的回调结构体，将其存入全局m_callbackArray数组
	TickReactor::iterator _begin = this->m_TickReactorMap.begin();
	TickReactor::iterator _end = this->m_TickReactorMap.upper_bound(_now.UI64());
	GE::Uint32 _cnt = 0;
	while(_begin != _end)
	{
		if (_cnt == MAX_CALLBACK_PER_DRIVER)
		{
			GE_WARN(0 == "too many pycallback in one second.");
			break;
		}
		m_callbackArray[_cnt] = _begin->second;
		++_cnt;
		this->m_TickReactorMap.erase(_begin++);
	}
	// 依次调用各个回调结构体
	GE_ITER_UI32(_idx, _cnt)
	{

		if (this->m_pyOwner.IsNone())
		{
			/*
			Return value: New reference.
			Call a callable Python object callable, with a variable number of PyObject* arguments. The arguments are provided as a variable number of parameters followed by NULL.
			Returns the result of the call on success, or NULL on failure.
			*/
			PyObject* pyResult_NewRef = PyObject_CallFunctionObjArgs(m_callbackArray[_idx].m_callback, Py_None, m_callbackArray[_idx].m_param, NULL);
			if (NULL == pyResult_NewRef)
			{
				PyErr_Print();
			}
			else
			{
				Py_DECREF(pyResult_NewRef);
			}
		}
		else
		{
			/*
			Return value: New reference.
			Call a callable Python object callable, with a variable number of PyObject* arguments. The arguments are provided as a variable number of parameters followed by NULL.
			Returns the result of the call on success, or NULL on failure.
			*/
			PyObject* pyResult_NewRef = PyObject_CallFunctionObjArgs(m_callbackArray[_idx].m_callback, this->m_pyOwner.GetObj_BorrowRef(), Py_None, m_callbackArray[_idx].m_param, NULL);
			if (NULL == pyResult_NewRef)
			{
				PyErr_Print();
			}
			else
			{
				Py_DECREF(pyResult_NewRef);
			}
		}
		Py_DECREF(m_callbackArray[_idx].m_callback);
		Py_DECREF(m_callbackArray[_idx].m_param);
	}
}

//////////////////////////////////////////////////////////////////////////
// GESmallTick
//////////////////////////////////////////////////////////////////////////
GESmallTick::GESmallTick( void )
	: m_LowerID(1)
	, m_BaseTime(static_cast<GE::Int32>(GEDateTime::Instance()->Seconds()))
{
}

GESmallTick::GESmallTick( PyObject* pPyOwner_BorrowRef )
	: m_LowerID(1)
	, m_BaseTime(static_cast<GE::Int32>(GEDateTime::Instance()->Seconds()))
{
	// 注意，这里传进来的参数是借用，需要保存则要增加引用计数
	this->m_pyOwner.SetObj_BorrowRef(pPyOwner_BorrowRef);
}

GESmallTick::~GESmallTick( void )
{
	// 这里释放还没有触发的Tick
	TickReactor::iterator _iter = this->m_TickReactorMap.begin();
	while(_iter != this->m_TickReactorMap.end())
	{
		Py_DECREF(_iter->second.m_callback);
		Py_DECREF(_iter->second.m_param);
		this->m_TickReactorMap.erase(_iter++);
	}
	this->m_TickReactorMap.clear();
}

void GESmallTick::SetOwner( PyObject* pPyOwner_BorrowRef )
{
	// 注意，这里传进来的参数是借用，需要保存则要增加引用计数
	this->m_pyOwner.SetObj_BorrowRef(pPyOwner_BorrowRef);
}


GE::Int32 GESmallTick::RegTick( GE::Uint32 uTimeSec, PyObject* pyCallBack_BorrowRef, PyObject* pyParam_BorrowRef )
{
	GE::Uint32 uCnt = 0;
	GE::Int32 DTime = 0;
	GE::Int32 ID = 0;
	for (; uCnt <= 5; ++uCnt)
	{
		//累进ID
		++this->m_LowerID;
		if (this->m_LowerID >= SMALL_TICK_LOW)
		{
			this->m_LowerID = 1;
		}
		// 计算触发相对时间
		DTime = static_cast<GE::Int32>(GEDateTime::Instance()->Seconds()) - this->m_BaseTime + static_cast<GE::Int32>(uTimeSec);
		// 构建Tick
		ID = DTime * SMALL_TICK_LOW + this->m_LowerID;
#if WIN
		GE_ERROR(static_cast<GE::Int32>(GEDateTime::Instance()->Seconds()) >=  this->m_BaseTime);
		GE_ERROR(DTime < SMALL_TICK_SPE);
#endif
		// 找到合适的ID，返回之
		if (this->m_TickReactorMap.find(ID) != this->m_TickReactorMap.end())
		{
			break;
		}
	}
	// 如果不能找到一个合适的ID，返回0
	if (uCnt == 5)
	{
		return 0;
	}
	// 增加Python的引用计数
	Py_INCREF(pyCallBack_BorrowRef);
	Py_INCREF(pyParam_BorrowRef);
	// 存入Map
	_PyCallBack PCB(pyCallBack_BorrowRef, pyParam_BorrowRef);
	this->m_TickReactorMap.insert(std::make_pair(ID, PCB));
	// 返回一个唯一ID
	return ID;
}

bool GESmallTick::UnregTick( GE::Int32 ID )
{
	TickReactor::iterator _iter = this->m_TickReactorMap.find(ID);
	// 根据ID查找回调结构体
	if (_iter == this->m_TickReactorMap.end())
	{
		return false;
	}
	// 减少引用计数
	Py_DECREF(_iter->second.m_callback);
	Py_DECREF(_iter->second.m_param);
	this->m_TickReactorMap.erase(_iter);
	return true;
}

bool GESmallTick::TriggerTick( GE::Int32 ID )
{
	return this->TriggerTick(ID, Py_None);
}

bool GESmallTick::TriggerTick( GE::Int32 ID, PyObject* pyTrigger_BorrowRef )
{
	TickReactor::iterator _iter = this->m_TickReactorMap.find(ID);
	// 根据ID查找回调结构体
	if (_iter == this->m_TickReactorMap.end())
	{
		return false;
	}
	// 注意这里要拷贝一份再从Map中删除
	_PyCallBack PC = _iter->second;
	// 先从Map中删除,防止在下面的函数调用中修改_iter
	this->m_TickReactorMap.erase(_iter);
	/*
	Return value: New reference.
	Call a callable Python object callable, with a variable number of PyObject* arguments. The arguments are provided as a variable number of parameters followed by NULL.
	Returns the result of the call on success, or NULL on failure.
	*/
	PyObject* pyResult_NewRef = PyObject_CallFunctionObjArgs(PC.m_callback, this->m_pyOwner.GetObj_BorrowRef(), pyTrigger_BorrowRef, PC.m_param, NULL);
	if (NULL == pyResult_NewRef)
	{
		PyErr_Print();
	}
	else
	{
		Py_DECREF(pyResult_NewRef);
	}
	// 减少引用计数
	Py_DECREF(PC.m_callback);
	Py_DECREF(PC.m_param);
	return true;
}

void GESmallTick::CallPerSecond()
{
	// 累进低位置0
	//this->m_LowerID = 0;
	// 没有Tick，直接返回之
	if (0 == this->m_TickReactorMap.size())
	{
		return;
	}
	// 一个全局静态的回调结构体数组的缓存
	static _PyCallBack SmallCallbackArray[MAX_CALLBACK_PER_DRIVER];
	// 计数现在的超时时间
	GE::Int32 _now = static_cast<GE::Int32>(GEDateTime::Instance()->Seconds()) - this->m_BaseTime + 1;
	_now *= SMALL_TICK_LOW;
	// 查找已经超时的回调结构体，将其存入全局m_callbackArray数组
	TickReactor::iterator _begin = this->m_TickReactorMap.begin();
	TickReactor::iterator _end = this->m_TickReactorMap.upper_bound(_now);
	GE::Uint32 _cnt = 0;
	while(_begin != _end)
	{
		if (_cnt == MAX_CALLBACK_PER_DRIVER)
		{
			GE_WARN(0 == "too many pycallback in one second.");
			break;
		}
		SmallCallbackArray[_cnt] = _begin->second;
		++_cnt;
		this->m_TickReactorMap.erase(_begin++);
	}
	// 依次调用各个回调结构体
	GE_ITER_UI32(_idx, _cnt)
	{
		_PyCallBack& PC = SmallCallbackArray[_idx];
		/*
		Return value: New reference.
		Call a callable Python object callable, with a variable number of PyObject* arguments. The arguments are provided as a variable number of parameters followed by NULL.
		Returns the result of the call on success, or NULL on failure.
		*/
		PyObject* pyResult_NewRef = PyObject_CallFunctionObjArgs(PC.m_callback, this->m_pyOwner.GetObj_BorrowRef(), Py_None, PC.m_param, NULL);
		if (NULL == pyResult_NewRef)
		{
			PyErr_Print();
		}
		else
		{
			Py_DECREF(pyResult_NewRef);
		}
		Py_DECREF(PC.m_callback);
		Py_DECREF(PC.m_param);
	}
}

PyObject* GESmallTick::GetFun_BorrowRef( GE::Int32 ID )
{
	TickReactor::iterator _iter = this->m_TickReactorMap.find(ID);
	// 根据ID查找回调结构体
	if (_iter == this->m_TickReactorMap.end())
	{
		return NULL;
	}
	else
	{
		return _iter->second.m_callback;
	}
}


GEFastTick::GEFastTick()
	: m_uTriggerSeconds(0)
{

}

GEFastTick::~GEFastTick()
{
	GE_ITER_UI16(uIdx, FAST_TICK_ROUND)
	{
		TickVector& TV = this->m_TickRoundArray[uIdx];
		GE_ITER_UI16(i, TV.size())
		{
			_PyCallBack& PCB = TV[i];
			if (PCB.m_callback)
			{
				Py_DECREF(PCB.m_callback);
				Py_DECREF(PCB.m_param);
			}
		}
	}
}

GE::Int64 GEFastTick::RegTick( GE::Uint32 uTimeSec, PyObject* pyCallBack_BorrowRef, PyObject* pyParam_BorrowRef )
{
	// 同一秒触发不能太长
	GE::Uint32 uOverTime = GEDateTime::Instance()->Seconds() + uTimeSec;
	TickVector& TV = this->m_TickRoundArray[uOverTime % FAST_TICK_ROUND];
	if (TV.size() >= MAX_CALLBACK_PER_DRIVER)
	{
		GE_WARN(0);
		return 0;
	}
	// 构建ID
	GE::B8 ID(static_cast<GE::Uint32>(TV.size()), uOverTime);
	// 加入队列
	Py_INCREF(pyCallBack_BorrowRef);
	Py_INCREF(pyParam_BorrowRef);
	_PyCallBack PCB(pyCallBack_BorrowRef, pyParam_BorrowRef);
	TV.push_back(PCB);
	// 返回ID
	return ID.I64();
}

bool GEFastTick::UnregTick( GE::Int64 uID )
{
	GE::B8& ID = GE_AS_B8(uID);
	// 已经触发过
	if (ID.UI32_1() <= this->m_uTriggerSeconds)
	{
		return false;
	}
	// 居然超出下标
	TickVector& TV = this->m_TickRoundArray[ID.UI32_1() % FAST_TICK_ROUND];
	if (ID.UI32_0() >= TV.size())
	{
		GE_WARN(0);
		return false;
	}
	// 释放指针，标记为NULL
	_PyCallBack& PCB = TV[ID.UI32_0()];
	Py_DECREF(PCB.m_callback);
	Py_DECREF(PCB.m_param);
	PCB.m_callback = NULL;
	PCB.m_param = NULL;
	return true;
}

void GEFastTick::CallPerSecond()
{
	// 标记触发状态
	this->m_uTriggerSeconds = GEDateTime::Instance()->Seconds();
	// 一个全局静态的回调结构体数组的缓存
	static _PyCallBack FastCallbackArray[MAX_CALLBACK_PER_DRIVER];
	// 当前触发列表
	TickVector& TV = this->m_TickRoundArray[this->m_uTriggerSeconds % FAST_TICK_ROUND];
	if (TV.empty())
	{
		return;
	}
	GE::Uint16 uSize = static_cast<GE::Uint16>(TV.size());
	GE_ERROR(uSize <= MAX_CALLBACK_PER_DRIVER);
	// 拷贝需要触发的回调结构体
	GE_ITER_UI32(_idx, uSize)
	{
		_PyCallBack& PCB = TV[_idx];
		FastCallbackArray[_idx] = PCB;
		PCB.m_callback = NULL;
		PCB.m_param = NULL;
	}
	// 清理回调列表（这里在触发回调之前必须先清理vector）
	TV.clear();
	// 依次调用各个回调结构体
	GE_ITER_UI32(_idx, uSize)
	{
		_PyCallBack& PCB = FastCallbackArray[_idx];
		// 有可能是被删除了的fast tick
		if (NULL == PCB.m_callback)
		{
			continue;
		}
		/*
		Return value: New reference.
		Call a callable Python object callable, with a variable number of PyObject* arguments. The arguments are provided as a variable number of parameters followed by NULL.
		Returns the result of the call on success, or NULL on failure.
		*/
		PyObject* pyResult_NewRef = PyObject_CallFunctionObjArgs(PCB.m_callback, PCB.m_param, NULL);
		if (NULL == pyResult_NewRef)
		{
			PyErr_Print();
		}
		else
		{
			Py_DECREF(pyResult_NewRef);
		}
		Py_DECREF(PCB.m_callback);
		Py_DECREF(PCB.m_param);
	}
}
