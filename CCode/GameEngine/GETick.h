/************************************************************************
Tick系统，支持延时执行python函数
TriggerTick有了触发参数，回调函数系统也支持。
************************************************************************/
#pragma once
#include <vector>
#include <map>
#include "GEInteger.h"
#include "GEPython.h"

// fast tick的最大延迟时间
#define FAST_TICK_ROUND 60

// tick的回调结构体
struct _PyCallBack
{
	_PyCallBack () {}
	_PyCallBack (PyObject* callback, PyObject* param)
		: m_callback(callback)
		, m_param(param)
	{
	}
	PyObject*	m_callback;			//回调函数
	PyObject*	m_param;			//回调参数
};

class GETick
{
	typedef std::map<GE::Int64, _PyCallBack>		TickReactor;
public:
	GETick(void);
	GETick(PyObject* pPyOwner_BorrowRef);
	virtual ~GETick(void);

public:
	GE::Int64				RegTick(GE::Uint32 uTimeSec, PyObject* pyCallBack_BorrowRef, PyObject* pyParam_BorrowRef);	//注册一个Tick
	bool					UnregTick(GE::Int64 uID);																	//取消一个tick
	bool					TriggerTick(GE::Int64 uID);																	//强制触发一个Tick
	bool					TriggerTick(GE::Int64 uID, PyObject* pyTrigger_BorrowRef);									//强制触发一个Tick
	void					CallPerSecond();																			//每秒调用一次，驱动tick

	GEPython::Object&		PyOwner() {return m_pyOwner;}

private:
	GEPython::Object		m_pyOwner;						//Tick的拥有者Python对象
	GE::B4					m_uLowerID;						//当前分配到的ID的低32位
	TickReactor				m_TickReactorMap;				//存储tick的Map
};

class GESmallTick
{
	typedef std::map<GE::Int32, _PyCallBack>		TickReactor;
public:
	GESmallTick(void);
	GESmallTick(PyObject* pPyOwner_BorrowRef);
	virtual ~GESmallTick(void);
	void					SetOwner(PyObject* pPyOwner_BorrowRef);

public:
	GE::Int32				RegTick(GE::Uint32 uTimeSec, PyObject* pyCallBack_BorrowRef, PyObject* pyParam_BorrowRef);	//注册一个Tick
	bool					UnregTick(GE::Int32 ID);																	//取消一个tick
	bool					TriggerTick(GE::Int32 ID);																	//强制触发一个Tick
	bool					TriggerTick(GE::Int32 ID, PyObject* pyTrigger_BorrowRef);									//强制触发一个Tick
	void					CallPerSecond();																			//每秒调用一次，驱动tick
	PyObject*				GetFun_BorrowRef(GE::Int32 ID);																//根据TickID，获取回调函数

	GEPython::Object&		PyOwner() {return m_pyOwner;}

private:
	GEPython::Object		m_pySelf;						//Tick的Python对象
	GEPython::Object		m_pyOwner;						//Tick的拥有者Python对象
	GE::Int32				m_LowerID;						//当前分配到的ID的累进数据
	GE::Int32				m_BaseTime;						//当前Tick的基本时间
	TickReactor				m_TickReactorMap;				//存储tick的Map
};

class GEFastTick
{
	typedef std::vector<_PyCallBack>				TickVector;
public:
	GEFastTick();
	~GEFastTick();

public:
	GE::Int64				RegTick(GE::Uint32 uTimeSec, PyObject* pyCallBack_BorrowRef, PyObject* pyParam_BorrowRef);	//注册一个Tick
	bool					UnregTick(GE::Int64 uID);																	//取消一个tick
	void					CallPerSecond();																			//每秒调用一次，驱动tick

private:
	TickVector				m_TickRoundArray[FAST_TICK_ROUND];
	GE::Uint32				m_uTriggerSeconds;
};

