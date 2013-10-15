/************************************************************************
网络层
************************************************************************/
#pragma once
#include <vector>
#include <boost/thread.hpp>

#include "GENetDefine.h"
#include "GENetConnectMgr.h"

class GENetWork
{
	GE_DISABLE_BOJ_CPY(GENetWork);
	typedef boost::asio::ip::tcp::acceptor				tyBoostAcceptor;
	typedef boost::thread								tyBoostThread;
	typedef std::vector<tyBoostThread*>					tyBoostThreads;

public:
	GENetWork(GE::Uint32 uMaxConnect, GE::Uint16 uThread);
	~GENetWork(void);

public:
	/*
	下面函数的后缀_L标识该函数只能在逻辑线程调用，否则会有线程问题。
	*/
	// 设置新的连接的相关参数
	void				SetParam_L(const ConnectParam& CP);
	// 监听一个端口
	bool				Listen_L(GE::Uint32 uListenPort);
	// 开始网络层工作
	void				Start_L();
	// 结束网络层的工作
	void				Stop_L();
	// 设置发送模式
	void				SetSendModel(bool bThreadSend);
	// 连接一个远程ip、端口
	bool				Connect_L(const char* sIP, GE::Uint32 uPort, GE::Uint32& uSessionID, ConnectParam* pCP = NULL);
	// 断开一个连接
	void				DisConnect_L(GE::Uint32 uSessionID);
	// 清理一个连接
	void				ClearConnect_L(GE::Uint32 uSessionID);
	// 设置一个连接消息重定向
	void				SetConnectForward_L(GE::Uint32 uSessionID);
	// 设置连接信息
	void				SetConnectInfo_L(GE::Uint32 uSessionID, GE::Uint16 uWho, void* pySessionID, void* pyWho);
	// 是否有某个连接
	bool				HasConnect(GE::Uint32 uSessionID);
	// 给uSessionID连接发送一段字节（线程不安全，只能逻辑线程调用）
	void				SendBytes_L(GE::Uint32 uSessionID, const void* pHead, GE::Uint16 uSize);
	// 给uSessionID连接发送一段字节（线程安全）
	void				SendBytes(GE::Uint32 uSessionID, const void* pHead, GE::Uint16 uSize);
	// 给多个连接发送一段字节（线程安全）
	void				BroadBytes(GE::Uint16 uWho, const void* pHead, GE::Uint16 uSize);
	// 是否网络层还在运行中
	bool				IsRun() {return m_bIsRun;}
	// 是否数据发送完毕
	bool				IsSendOver_L(GE::Uint32 uSessionID);
	// 线程发送消息
	bool				ThreadSend() {return m_bThreadSend;}

	// 获取消息的函数簇
	bool				MoveNextMsg();
	GE::Uint32			CurSessionID() {return m_pCurConnect->SessionID();}
	GE::B8&				CurB8() {return m_pCurConnect->BindB8();}
	GE::Uint16&			CurWho() {return m_pCurConnect->Who();}
	MsgBase*			CurMsg() {return m_pCurMsg;}
	bool				CurIsClose() {return m_bLastConnectClose;}
	bool				CurIsRound() {return m_uIterCnt == 0;}
	GE::Uint16			CurState() {return m_pCurConnect->State();}
	void				CurRemoteEndPoint(std::string& sIP, GE::Uint32& uPort) {m_pCurConnect->RemoteEndPoint(sIP, uPort);}
	void*&				CurPySessionID() {return m_pCurConnect->PySessionID();}
	void*&				CurPyWho() {return m_pCurConnect->PyWho();}

public:
	BoostIOService&		IOS() {return m_IOS;}

private:
	/*
	下面函数的后缀_N标识该函数只能在网络线程程调用，否则会有线程问题。
	*/
	// 异步接收连接
	void				AsyncAccept_N();
	void				HandleAccept_N(GENetConnect::ConnectSharedPtr spConnect, const boost::system::error_code& error);
	void				BoostAsioRun();
	void				ThreadSendFun();

private:
	// 连接管理器、线程锁
	GENetConnectMgr		m_ConnectMgr;
	boost::mutex		m_ConnectMutex;
	// boost的asio相关
	bool				m_bIsRun;
	BoostIOService		m_IOS;
	tyBoostAcceptor*	m_pAcceptor;
	tyBoostThreads		m_pNetWorkThreads;
	bool				m_bThreadSend;
	tyBoostThread*		m_pThreadSend;
	// 新的连接的参数
	ConnectParam		m_ConnectParam;
	// 当前消息的缓存
	GENetConnect*		m_pCurConnect;
	MsgBase*			m_pCurMsg;
	GE::Uint32			m_uIterCnt;
	bool				m_bLastConnectClose;
	// 统计信息
	GE::Uint32			m_uSta_LastSessionID;
	GE::Uint32			m_uSta_MsgCnt;
	GE::Uint32			m_uSta_MaxConnect;
};

