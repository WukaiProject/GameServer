/************************************************************************
网络连接
************************************************************************/
#pragma once
#include <string>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include "GENetDefine.h"
#include "GENetSendBuf.h"
#include "GENetRecvBuf.h"
#include "GENetMessage.h"

class GENetWork;
class GENetSession;

// 一个连接的状态
enum NetConnectState
{
	enNetConnect_Work,
	enNetConnect_RemoteClose,
	enNetConnect_MsgError,
	enNetConnect_SendBufFull,
	enNetConnect_RecvBufFull,
	enNetConnect_ConnectFull,
	enNetConnect_LocalClose
};

typedef boost::asio::io_service				BoostIOService;
typedef boost::asio::ip::tcp::socket		BoostSocket;

// 网络连接类
class GENetConnect
	: public boost::enable_shared_from_this<GENetConnect>
{
	GE_DISABLE_BOJ_CPY(GENetConnect);
public:
	typedef boost::shared_ptr<GENetConnect>		ConnectSharedPtr;

	GENetConnect(GENetWork* pNetWork, ConnectParam& CP);
	~GENetConnect();

public:
	GE::Uint32			SessionID() {return m_uSessionID;}									//获取该连接的SessionID
	void				SessionID(GE::Uint32 uSessionID) {m_uSessionID = uSessionID;}		//设置该连接的SessionID
	GE::Uint32			ConnectTime() {return m_uConnectTime;}								//获取该连接的创建时间
	GE::B8&				BindB8() {return m_pBindB8;}										//绑定该连接的绑定的数据
	BoostSocket&		Socket() {return m_BoostSocket;}									//该连接的boost::socket
	void				RemoteEndPoint(std::string& sIP, GE::Uint32& uPort);				//获取该连接的远程IP、端口
	GE::Uint16			State() {return static_cast<GE::Uint16>(m_State);}					//连接的当前状态
	GE::Uint16&			Who() {return m_uWho;}												//当前连接的类型Who
	void*&				PySessionID() {return m_pSessionID;}								//当前连接的SessionID的Python缓存
	void*&				PyWho() {return m_pWho;}											//当前连接的Who的Python缓存
	void				NeedTGWHeadSize(GE::Uint16 uSize) {m_uNeedTGWHeadSize = uSize;}		//设置还需TGW换行符数量

	void				Start();
	void				Shutdown(NetConnectState state);									//因为某个原因关闭该连接
	bool				IsShutdown();														//该连接是否关闭
	void				SendBytes(const void* pHead, GE::Uint16 uSize);						//向该连接发送数据
	bool				ReadMsg(MsgBase** pMsg);											//从该连接中读取一条网络消息
	bool				IsSendOver() {return this->m_SendBuf.IsEmpty();}					//改连接的发送buf是否已空
	void				SetForward();														//当前连接需要重定向消息
	void				ThreadAsyncSendBlock();												//外部驱动发送消息

private:
	void				AsyncRecvTGWHead();													//异步接收腾讯网关包头
	void				AsyncRecvHead();													//异步接收消息头
	void				AsyncRecvBody();													//异步接收消息体
	void				AsyncSendBlock();													//异步发送消息
	void				HandlReadTGWHead(const boost::system::error_code& errorCode, size_t uTransferredBytes);		//完成腾讯网关包头的回调
	void				HandleReadMsgHead(const boost::system::error_code& errorCode, size_t uTransferredBytes);	//完成消息头的接收的回调
	void				HandleReadMsgBody(const boost::system::error_code& errorCode, size_t uTransferredBytes);	//完成消息体的接收的回调
	void				HandleWriteMsg(const boost::system::error_code& errorCode, size_t uTransferredBytes);		//完成消息发送的回调
	bool				RecvCompletionMsg();																		//接收了一条完整的消息

private:
	GENetWork*			m_pNetWork;
	GE::Uint32			m_uSessionID;		//连接SessionID
	GE::Uint32			m_uConnectTime;		//该连接的创建时间

	GENetSendBuf		m_SendBuf;			//连接的发送buf
	GENetRecvBuf		m_RecvBuf;			//连接的接收buf
	GENetBuf			m_RecvCache;		//连接的当前消息接收buf
	boost::mutex		m_SendMutex;		//发送buf的线程锁
	boost::mutex		m_RecvMutex;		//接收buf的线程锁

	NetConnectState		m_State;			//连接的状态
	BoostSocket			m_BoostSocket;		//连接的boost::socket
	GE::B8				m_pBindB8;			//连接绑定的8字节信息
	GE::Uint16			m_uWho;				//连接的类型
	GE::Uint16			m_uNeedTGWHeadSize;	//还需接收TGW包头换行符数量
	GE::Uint16			m_uTGWCharSize;		//接收的TGW包头字符数量
	/*
	这里要注意，不要和Python有什么联系，多线程环境下处理Python虚拟机太繁杂。
	只有下面这两变量不会变化才可以做缓存。
	*/
	void*				m_pSessionID;		//连接的SessionID的Python缓存
	void*				m_pWho;				//连接的Who的Python缓存

	bool				m_bIsForward;		//该连接的消息是否要重定向到某个连接
};


