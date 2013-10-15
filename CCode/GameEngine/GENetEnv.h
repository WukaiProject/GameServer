/************************************************************************
网络层的全局数据
************************************************************************/
#pragma once
#include "GEInteger.h"
#include "GESingleton.h"

class GENetEnv
	: public GESingleton<GENetEnv>
{
public:
	GENetEnv()
		: m_bIsStable(true), m_bIsForward(false)
		, m_uForwardSessionID(MAX_UINT32)
		, m_uTGWMaxSize(0)

		, m_bCanCombineMsg(false)
	{}
	~GENetEnv() {}

public:
	/*
	消息导向功能和网络层拼包功能不能同时存在。
	网络层拼包更能要求只有一个线程发送消息，才可正确的在网络层拼包。
	而消息导向功能则会在网络线程中发送消息，故与网络拼包功能矛盾。
	*/
	bool					IsStable() {return m_bIsStable;}
	void					SetUnstable() {m_bIsStable = false;}

	bool					IsForward() {return m_bIsForward;}
	void					SetCanForward() {m_bIsForward = true; GE_ERROR(!m_bCanCombineMsg);}
	GE::Uint32&				ForwardSessionID() {return m_uForwardSessionID;}

	bool					CanCombineMsg() {return m_bCanCombineMsg;}
	void					SetCanCombineMsg() {m_bCanCombineMsg = true; GE_ERROR(!m_bIsForward);}

	GE::Uint16				TGWMaxSize() {return this->m_uTGWMaxSize;}
	void					SetTGWMaxSize(GE::Uint16 uSize) {this->m_uTGWMaxSize = uSize;}

private:
	bool					m_bIsStable;				//消息导向连接是否稳定
	bool					m_bIsForward;				//是否开启消息导向
	GE::Uint32				m_uForwardSessionID;		//消息导向连接SessionID

	bool					m_bCanCombineMsg;			//能否网络层拼包
	GE::Uint16				m_uTGWMaxSize;				//TGW换行符的格式
};

