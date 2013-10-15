/*我是UTF8无签名编码*/
#include <boost/bind.hpp>
#include "GENetConnect.h"
#include "GENetWork.h"
#include "GENetEnv.h"
#include "GEIO.h"
#include "GEDateTime.h"

#define TGW_MAX_SIZE			3

GENetConnect::GENetConnect( GENetWork* pNetWork, ConnectParam& CP )
	: m_pNetWork(pNetWork)
	, m_uSessionID(MAX_UINT32)
	, m_SendBuf(CP.uSendBlockSize, CP.uSendBlockNum)
	, m_RecvBuf(CP.uRecvBlockSize, CP.uRecvBlockNum)
	, m_RecvCache(CP.uRecvBlockSize)
	, m_BoostSocket(pNetWork->IOS())
	, m_State(enNetConnect_Work)
	, m_uWho(0)
	, m_uNeedTGWHeadSize(0)
	, m_uTGWCharSize(0)
	, m_pSessionID(NULL)
	, m_pWho(NULL)
	, m_bIsForward(false)
{
	m_uConnectTime = GEDateTime::Instance()->Seconds();
}

GENetConnect::~GENetConnect()
{
	this->m_pNetWork = NULL;
}

void GENetConnect::RemoteEndPoint( std::string& sIP, GE::Uint32& uPort )
{
	sIP = this->m_BoostSocket.remote_endpoint().address().to_string();
	uPort = this->m_BoostSocket.remote_endpoint().port();
}

void GENetConnect::Start()
{
	if (this->m_uNeedTGWHeadSize > 0)
	{
		this->AsyncRecvTGWHead();
	}
	else
	{
		this->AsyncRecvHead();
	}
}

void GENetConnect::Shutdown(NetConnectState state)
{
	/*
	因为网络线程和逻辑主线程都有可能关闭socket，如果同时关闭则会导致崩溃。
	*/
	if (!this->IsShutdown())
	{
		try
		{
			// 关闭socket
			this->m_State = state;
			this->m_BoostSocket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
#if WIN
			this->m_BoostSocket.close();
#elif LINUX
			this->m_BoostSocket.cancel();
#endif
		}
		catch (std::exception& e)
		{
			e.what();
			//GE_OUT<<e.what()<<GE_END;
		}
	}
}

bool GENetConnect::IsShutdown()
{
	return enNetConnect_Work != this->m_State;
}

void GENetConnect::SendBytes(const void* pHead, GE::Uint16 uSize)
{
	if (this->IsShutdown())
	{
		return;
	}
	// 对m_SendBuf的操作要加线程锁
	this->m_SendMutex.lock();
	// 向字节块中写入字节
	bool ret = this->m_SendBuf.WriteBytes(pHead, uSize);
	this->m_SendMutex.unlock();
	// 如果不能再向发送buf中写入数据，说明发送buf满了，断掉这个连接
	if (!ret)
	{
		GE_EXC<<"session("<<this->SessionID()<<") send msg buf is full."<<GE_END;
		this->Shutdown(enNetConnect_SendBufFull);
		return;
	}

	if (!this->m_pNetWork->ThreadSend())
	{
		this->AsyncSendBlock();
	}
}

bool GENetConnect::ReadMsg( MsgBase** pMsg )
{
	// 这里readMsg只有逻辑线程会调用，这里不要加锁
	if (this->m_RecvBuf.ReadMsgFromReadBuf(pMsg))
	{
		return true;
	}
	else
	{
		// MoveToNextReadBuf要加线程锁
		this->m_RecvMutex.lock();
		bool ret = this->m_RecvBuf.MoveToNextReadBuf();
		this->m_RecvMutex.unlock();
		if (ret)
		{
			return this->m_RecvBuf.ReadMsgFromReadBuf(pMsg);
		}
		else
		{
			return false;
		}
	}
}

void GENetConnect::SetForward()
{
	if (GENetEnv::Instance()->IsForward())
	{
		this->m_bIsForward = true;
	}
	else
	{
		GE_EXC<<"set forward error with no forward ENV."<<GE_END;
	}
}

void GENetConnect::ThreadAsyncSendBlock()
{
	this->AsyncSendBlock();
}

void GENetConnect::AsyncRecvTGWHead()
{
	if (this->IsShutdown())
	{
		return;
	}
	m_RecvCache.Reset();
	boost::asio::async_read(m_BoostSocket,
		boost::asio::buffer(m_RecvCache.HeadPtr(), 1),
		boost::bind(&GENetConnect::HandlReadTGWHead, shared_from_this(),
		boost::asio::placeholders::error,
		boost::asio::placeholders::bytes_transferred));
}

void GENetConnect::AsyncRecvHead()
{
	if (this->IsShutdown())
	{
		return;
	}
	// 这里明确的重置当前消息接收buf
	m_RecvCache.Reset(MSG_FORWARD_SIZE);
	boost::asio::async_read(m_BoostSocket,
		boost::asio::buffer(m_RecvCache.HeadPtr(MSG_FORWARD_SIZE), sizeof(MsgBase)),
		boost::bind(&GENetConnect::HandleReadMsgHead, shared_from_this(),
		boost::asio::placeholders::error,
		boost::asio::placeholders::bytes_transferred));
}

void GENetConnect::AsyncRecvBody()
{
	if (this->IsShutdown())
	{
		return;
	}
	MsgBase* pMsg = static_cast<MsgBase*>(m_RecvCache.HeadPtr(MSG_FORWARD_SIZE));
	boost::asio::async_read(m_BoostSocket,
		boost::asio::buffer(m_RecvCache.WriteFence_us(), pMsg->Size() - sizeof(MsgBase)), //这里注意用正确的头指针和大小
		boost::bind(&GENetConnect::HandleReadMsgBody, shared_from_this(),
		boost::asio::placeholders::error,
		boost::asio::placeholders::bytes_transferred));
}

void GENetConnect::AsyncSendBlock()
{
	if (this->IsShutdown())
	{
		return;
	}
	void* pHead = NULL;
	GE::Uint16 uSize = 0;
	// 对m_SendBuf的操作要加线程锁
	this->m_SendMutex.lock();
	// 尝试从发送buf中拿出一块数据进行发送
	bool ret = this->m_SendBuf.HoldBlock(&pHead, uSize);
	this->m_SendMutex.unlock();
	// 如果从发送消息缓冲中hold不出数据，则不发送
	if (!ret)
	{
		return;
	}
	// 异步发送消息之
	boost::asio::async_write(m_BoostSocket,
		boost::asio::buffer(pHead, uSize),
		boost::bind(&GENetConnect::HandleWriteMsg, shared_from_this(),
		boost::asio::placeholders::error,
		boost::asio::placeholders::bytes_transferred));
}

void GENetConnect::HandlReadTGWHead( const boost::system::error_code& errorCode, size_t uTransferredBytes )
{
	if (this->IsShutdown())
	{
		return;
	}
	if (errorCode)
	{
		this->Shutdown(enNetConnect_RemoteClose);
		return;
	}
	/*
	腾讯平台上居然会用HTTP测试访问被监听的端口，故要检测TGW包头。
	*/
	static char tgw[] = {'t', 'g', 'w'};
	char head = *(static_cast<char*>(this->m_RecvCache.HeadPtr()));
	if (this->m_uTGWCharSize < sizeof(tgw))
	{
		if (head != tgw[this->m_uTGWCharSize])
		{
			this->Shutdown(enNetConnect_MsgError);
			return;
		}
	}
	else
	{
		if (head == '\n')
		{
			--m_uNeedTGWHeadSize;
		}
	}
	++m_uTGWCharSize;
	this->Start();
}

void GENetConnect::HandleReadMsgHead( const boost::system::error_code& errorCode, size_t uTransferredBytes )
{
	if (this->IsShutdown())
	{
		return;
	}
	if (errorCode)
	{
		this->Shutdown(enNetConnect_RemoteClose);
		return;
	}
	MsgBase* pMsg = static_cast<MsgBase*>(m_RecvCache.HeadPtr(MSG_FORWARD_SIZE));
	// 消息错误,长度小于
	if (pMsg->Size() < sizeof(MsgBase) || pMsg->Size() > m_RecvCache.CanWriteSize())
	{
#ifdef WIN
		GE_EXC<<"Error Msg Length, Type("<<pMsg->Type()<<") Size("<<pMsg->Size()<<")"<<GE_END;
#endif
		this->Shutdown(enNetConnect_MsgError);
		return;
	}
	// 这里明确说明已经接收了消息头，移动读指针
	m_RecvCache.MoveWriteFence_us(sizeof(MsgBase));
	/*
	如果消息只有消息头，则直接接收完成
	注意，此时的消息肯定是4的倍数
	*/
	if (pMsg->Size() == sizeof(MsgBase))
	{
		if (!this->RecvCompletionMsg())
		{
			this->Shutdown(enNetConnect_RecvBufFull);
			GE_EXC<<"session("<<this->SessionID()<<") recv msg buf is full."<<GE_END;
			return;
		}
		this->AsyncRecvHead();
	}
	else
	{
		this->AsyncRecvBody();
	}
}

void GENetConnect::HandleReadMsgBody( const boost::system::error_code& errorCode, size_t uTransferredBytes )
{
	if (this->IsShutdown())
	{
		return;
	}
	if (errorCode)
	{
		this->Shutdown(enNetConnect_RemoteClose);
		return;
	}
	MsgBase* pMsg = static_cast<MsgBase*>(m_RecvCache.HeadPtr(MSG_FORWARD_SIZE));
	// 注意，这里消息长度可能不是字节对齐的，要进行修正
	pMsg->Align();
	// 这里明确说明已经接收了消息体，移动读指针
	m_RecvCache.MoveWriteFence_us(pMsg->Size() - sizeof(MsgBase));
	// 如果不能写入，说明接收消息缓存满了，断开连接
	if (!this->RecvCompletionMsg())
	{
		this->Shutdown(enNetConnect_RecvBufFull);
		GE_EXC<<"session("<<this->SessionID()<<") recv msg buf is full."<<GE_END;
		return;
	}
	// 继续接收消息头
	this->AsyncRecvHead();
}

void GENetConnect::HandleWriteMsg( const boost::system::error_code& errorCode, size_t uTransferredBytes )
{
	// 对m_SendBuf的操作要加线程锁
	this->m_SendMutex.lock();
	// 首先要归还hold住的发送字节块
	this->m_SendBuf.ReleaseBlock();
	this->m_SendMutex.unlock();

	if (this->IsShutdown())
	{
		return;
	}
	if (errorCode)
	{
		this->Shutdown(enNetConnect_RemoteClose);
		return;
	}

	if (!this->m_pNetWork->ThreadSend())
	{
		this->AsyncSendBlock();
	}
}

bool GENetConnect::RecvCompletionMsg( )
{
	MsgBase* pMsg = static_cast<MsgBase*>(m_RecvCache.HeadPtr(MSG_FORWARD_SIZE));
#ifdef WIN
	if (this->m_bIsForward)
	{
		if (pMsg->Type() == enGEMsg_Forward_To || pMsg->Type() == enGEMsg_Forward_Other)
		{
			GE_ERROR(0);
		}
	}
#endif
	// 如果要重定向消息，重定向之
	if (this->m_bIsForward)
	{
		if (GENetEnv::Instance()->ForwardSessionID() != MAX_UINT32)
		{
			MsgForwardFrom msg;
			msg.uSessionID = this->m_uSessionID;
			msg.AddSize(pMsg->Size());
			this->m_RecvCache.InsertBytes_us(&msg, sizeof(msg));
			MsgBase* pNMsg = static_cast<MsgBase*>(m_RecvCache.HeadPtr());
			/*
			这里根据ForwardSessionID()是否稳定来确定是否使用加锁的发送消息函数。
			以现在的网络层结构来看，只要ForwardSessionID()在下面调用的时候不会被删除就不须加速，否则需要加锁。
			*/
			if (GENetEnv::Instance()->IsStable())
			{
				this->m_pNetWork->SendBytes_L(GENetEnv::Instance()->ForwardSessionID(), pNMsg, pNMsg->Size());
			}
			else
			{
				this->m_pNetWork->SendBytes(GENetEnv::Instance()->ForwardSessionID(), pNMsg, pNMsg->Size());
			}
		}
		return true;
	}

	// 需要将这个消息拆出子消息，并转发之
	if (pMsg->Type() == enGEMsg_Forward_To && pMsg->Size() > sizeof(MsgForwardTo))
	{
		MsgForwardTo* pFMsg = static_cast<MsgForwardTo*>(pMsg);
		MsgBase* pNMsg = static_cast<MsgBase*>(static_cast<void*>((char*)pMsg + sizeof(MsgForwardTo)));
#ifdef WIN
		GE_ERROR(pFMsg->Size() == pNMsg->Size() + sizeof(MsgForwardTo));
#endif
		this->m_pNetWork->SendBytes(pFMsg->uSessionID, pFMsg, pFMsg->Size());
		return true;
	}

	// 需要将这个消息拆出子消息，并广播之
	if (pMsg->Type() == enGEMsg_Forward_Other && pMsg->Size() > sizeof(MsgForwardOther))
	{
		MsgForwardOther* pFMsg = static_cast<MsgForwardOther*>(pMsg);
		MsgBase* pNMsg = static_cast<MsgBase*>(static_cast<void*>((char*)pMsg + sizeof(MsgForwardOther)));
#ifdef WIN
		GE_ERROR(pFMsg->Size() == pNMsg->Size() + sizeof(MsgForwardTo));
#endif
		this->m_pNetWork->BroadBytes(static_cast<GE::Uint16>(pFMsg->uWho), pNMsg, pNMsg->Size());
		return true;
	}

	// 默认情况下存入buf
	// 对m_RecvBuf的操作要加线程锁
	this->m_RecvMutex.lock();
	// 尝试将消息些入接收消息buf
	bool ret = this->m_RecvBuf.WriteMsg(pMsg);
	this->m_RecvMutex.unlock();
	
	return ret;
}

