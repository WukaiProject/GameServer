/*我是UTF8无签名编码*/
#include <boost/lexical_cast.hpp>
#include "GENetWork.h"
#include "GENetEnv.h"
#include "GEIO.h"
#include "GEDateTime.h"

GENetWork::GENetWork( GE::Uint32 uMaxConnect, GE::Uint16 uThread )
	: m_ConnectMgr(uMaxConnect)
	, m_bIsRun(false)
	, m_pAcceptor(NULL)
	, m_bThreadSend(false)
	, m_pThreadSend(NULL)
	, m_pCurConnect(NULL)
	, m_pCurMsg(NULL)
	, m_bLastConnectClose(false)
{
	GE_ITER_UI32(uIdx, uThread)
	{
		this->m_pNetWorkThreads.push_back(NULL);
	}
}

GENetWork::~GENetWork(void)
{
	this->Stop_L();
	GE_SAFE_DELETE(m_pAcceptor);
	GE_ITER_UI32(uIdx, static_cast<GE::Uint32>(m_pNetWorkThreads.size()))
	{
		GE_SAFE_DELETE(m_pNetWorkThreads[uIdx]);
	}
	if (NULL != this->m_pThreadSend)
	{
		GE_SAFE_DELETE(m_pThreadSend);
	}
}

void GENetWork::SetParam_L( const ConnectParam& CP )
{
	m_ConnectParam = CP;
}

bool GENetWork::Listen_L( GE::Uint32 uListenPort )
{
	if (NULL == m_pAcceptor)
	{
		try
		{
			/*
			Win下，关闭进程马上会释放socket，进程可以重启马上再监听这个端口。
			如果开启SO_REUSEADDR选项，结果就是两进程可以同时监听一个端口而不报错。
			*/
#ifdef WIN
			this->m_pAcceptor = new tyBoostAcceptor(m_IOS, 
				boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::from_string("0.0.0.0"), uListenPort), false);
			/*
			Linux下，关闭进程不会马上是否socket，还有可恶的腾讯GTW造成的FIN_WAIT1问题，导致不能马上对端口进行监听。
			如果开启SO_REUSEADDR选项，可解决这些问题
			*/
#elif LINUX
			this->m_pAcceptor = new tyBoostAcceptor(m_IOS, 
				boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::from_string("0.0.0.0"), uListenPort), true);
#endif
			this->AsyncAccept_N();
		}
		catch (std::exception& e)
		{
			GE_OUT<<e.what()<<GE_END;
			GE_EXC<<"can't listen the port("<<uListenPort<<")."<<GE_END;
			return false;
		}
		return true;
	}
	else
	{
		GE_EXC<<"can not double listen."<<GE_END;
		return false;
	}
}

void GENetWork::Start_L()
{
	GE_ITER_UI32(uIdx, static_cast<GE::Uint32>(m_pNetWorkThreads.size()))
	{
		this->m_pNetWorkThreads[uIdx] = new tyBoostThread(boost::bind(&GENetWork::BoostAsioRun, this));
	}
	this->m_bIsRun = true;
}

void GENetWork::Stop_L()
{
	this->m_bIsRun = false;
	this->m_bThreadSend = false;
	// 断开所有的连接
	this->m_ConnectMutex.lock();
	GE_ITER_UI32(_idx, m_ConnectMgr.ConnectCnt())
	{
		m_ConnectMgr.DelConnect(m_ConnectMgr.NextConnect()->SessionID());
	}
	this->m_pCurConnect = NULL;
	this->m_ConnectMutex.unlock();
	// 停止asio运行
	this->m_IOS.stop();
	// 等待asio线程返回
	GE_ITER_UI32(uIdx, static_cast<GE::Uint32>(m_pNetWorkThreads.size()))
	{
		this->m_pNetWorkThreads[uIdx]->timed_join(boost::posix_time::seconds(5));
	}
	if (NULL != this->m_pThreadSend)
	{
		this->m_pThreadSend->timed_join(boost::posix_time::seconds(5));
	}
}

void GENetWork::SetSendModel( bool bThreadSend )
{
	if (this->m_bThreadSend == bThreadSend)
	{
		return;
	}
	this->m_bThreadSend = bThreadSend;
	if (bThreadSend)
	{
		if (NULL == this->m_pThreadSend)
		{
			this->m_pThreadSend = new tyBoostThread(boost::bind(&GENetWork::ThreadSendFun, this));
		}
	}
}

bool GENetWork::Connect_L( const char* sIP, GE::Uint32 uPort, GE::Uint32& uSessionID, ConnectParam* pCP /*= NULL*/ )
{
	if (NULL == pCP)
	{
		pCP = &m_ConnectParam;
	}

	boost::asio::ip::tcp::resolver resolver(m_IOS);
	boost::asio::ip::tcp::resolver::query query(sIP, boost::lexical_cast<std::string>(uPort));
	boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
	GENetConnect::ConnectSharedPtr spConnect = GENetConnect::ConnectSharedPtr(new GENetConnect(this, *pCP));
	try
	{
		boost::asio::connect(spConnect->Socket(), endpoint_iterator);
	}
	catch (std::exception& e)
	{
#ifdef WIN
		e.what();
		GE_EXC<<"can't connect the remote address. ip("<<sIP<<") port("<<uPort<<")."<<GE_END;
#else
		GE_EXC<<"can't connect the remote address. ip("<<sIP<<") port("<<uPort<<") what("<<e.what()<<")."<<GE_END;
#endif
		return false;
	}
	// 对连接管理器的操作要加锁
	this->m_ConnectMutex.lock();
	bool ret = this->m_ConnectMgr.AddConnect(spConnect, uSessionID);
	if (ret)
	{
		spConnect->Start();
		// 统计连接数
		this->m_uSta_LastSessionID = spConnect->SessionID();
		GE::Uint32 uConnectCnt = this->m_ConnectMgr.ConnectCnt();
		this->m_uSta_MaxConnect = GE_MAX(m_uSta_MaxConnect, uConnectCnt);
	}
	else
	{
		spConnect->Shutdown(enNetConnect_ConnectFull);
		GE_EXC<<"too much network connect("<<m_ConnectMgr.ConnectCnt()<<")."<<GE_END;

	}
	this->m_ConnectMutex.unlock();
	return ret;
}

void GENetWork::DisConnect_L( GE::Uint32 uSessionID )
{
	// 这里虽然从ConnectMgr内部取出来pConnect指针，并在后面使用了pConnect指针
	// 但是，对于pConnect的指针的释放全部在逻辑线程，只要保证可以取出指针，既可以保证pConnect指针的有效性
	GENetConnect* pConnect = this->m_ConnectMgr.FindConnect(uSessionID);
	if (pConnect)
	{
		pConnect->Shutdown(enNetConnect_LocalClose);
	}
}

void GENetWork::ClearConnect_L(GE::Uint32 uSessionID)
{
	/************************************************************************
	这里一定要先比较是否删除当前连接，因为一旦m_ConnectMgr.DelConnect(uSessionID)
	连接对象就释放了，无法再比较了。
	************************************************************************/
	bool bIsCloseCurConnect = false;
	if (m_pCurConnect && m_pCurConnect->SessionID() == uSessionID)
	{
		bIsCloseCurConnect = true;
	}
	// 对连接管理器的操作要加锁
	this->m_ConnectMutex.lock();
	bool ret = this->m_ConnectMgr.DelConnect(uSessionID);
	this->m_ConnectMutex.unlock();
	// 如果删除了这个连接，并且是当前连接则要把当前连接的指针置NULL
	if (ret && bIsCloseCurConnect)
	{
		this->m_pCurConnect = NULL;
	}
}

void GENetWork::SetConnectForward_L(GE::Uint32 uSessionID)
{
	// 这里虽然从ConnectMgr内部取出来pConnect指针，并在后面使用了pConnect指针
	// 但是，对于pConnect的指针的释放全部在逻辑线程，只要保证可以取出指针，既可以保证pConnect指针的有效性
	GENetConnect* pConnect = this->m_ConnectMgr.FindConnect(uSessionID);
	if (pConnect)
	{
		pConnect->SetForward();
	}
}

void GENetWork::SetConnectInfo_L(GE::Uint32 uSessionID, GE::Uint16 uWho, void* pySessionID, void* pyWho)
{
	// 这里虽然从ConnectMgr内部取出来pConnect指针，并在后面使用了pConnect指针
	// 但是，对于pConnect的指针的释放全部在逻辑线程，只要保证可以取出指针，既可以保证pConnect指针的有效性
	GENetConnect* pConnect = this->m_ConnectMgr.FindConnect(uSessionID);
	if (pConnect)
	{
		pConnect->Who() = uWho;
		pConnect->PySessionID() = pySessionID;
		pConnect->PyWho() = pyWho;
	}
	else
	{
		GE_EXC<<"can't find connect on SetConnectInfo_L.maybe memory lost."<<GE_END;
	}
}

bool GENetWork::HasConnect(GE::Uint32 uSessionID)
{
	// 这里不改变ConnectMgr内部数据，不用加锁
	return this->m_ConnectMgr.HasConnect(uSessionID);
}

void GENetWork::SendBytes_L( GE::Uint32 uSessionID, const void* pHead, GE::Uint16 uSize )
{
	// 这里虽然从ConnectMgr内部取出来pConnect指针，并在后面使用了pConnect指针
	// 但是，对于pConnect的指针的释放全部在逻辑线程，只要保证可以取出指针，既可以保证pConnect指针的有效性
	GENetConnect* pConnect = this->m_ConnectMgr.FindConnectForHasData(uSessionID);
	if (pConnect)
	{
		pConnect->SendBytes(pHead, uSize);
	}
}

void GENetWork::SendBytes( GE::Uint32 uSessionID, const void* pHead, GE::Uint16 uSize )
{
	// 开启了网络拼包功能不能多线程发送字节流
	if (GENetEnv::Instance()->CanCombineMsg())
	{
		GE_EXC<<"can't use SendBytes function on CanCombineMsg."<<GE_END;
		return;
	}
	// 这里相比之函数SendBytes_L，因为是多线程环境中，必须加锁
	this->m_ConnectMgr.Lock(uSessionID);
	GENetConnect* pConnect = this->m_ConnectMgr.FindConnectForHasData(uSessionID);
	if (pConnect)
	{
		pConnect->SendBytes(pHead, uSize);
	}
	this->m_ConnectMgr.Unlock(uSessionID);
}

void GENetWork::BroadBytes(GE::Uint16 uWho, const void* pHead, GE::Uint16 uSize)
{
	// 开启了网络拼包功能不能多线程发送字节流
	if (GENetEnv::Instance()->CanCombineMsg())
	{
		GE_EXC<<"can't use SendBytes function on CanCombineMsg."<<GE_END;
		return;
	}
	// 对连接管理器的迭代要加锁
	this->m_ConnectMutex.lock();
	GE_ITER_UI32(_idx, m_ConnectMgr.ConnectCnt())
	{
		GENetConnect* pConnect = this->m_ConnectMgr.NextConnect();
		if (pConnect->Who() == uWho)
		{
			pConnect->SendBytes(pHead, uSize);
		}
	}
	this->m_ConnectMutex.unlock();
}

bool GENetWork::IsSendOver_L( GE::Uint32 uSessionID )
{
	// 这里虽然从ConnectMgr内部取出来pConnect指针，并在后面使用了pConnect指针
	// 但是，对于pConnect的指针的释放全部在逻辑线程，只要保证可以取出指针，既可以保证pConnect指针的有效性
	GENetConnect* pConnect = this->m_ConnectMgr.FindConnect(uSessionID);
	if (pConnect)
	{
		return pConnect->IsSendOver();
	}
	else
	{
		return true;
	}
}

void GENetWork::AsyncAccept_N()
{
	if (NULL == this->m_pAcceptor)
	{
		return;
	}
	GENetConnect::ConnectSharedPtr spConnect = GENetConnect::ConnectSharedPtr(new GENetConnect(this, this->m_ConnectParam));
	this->m_pAcceptor->async_accept(spConnect->Socket(), 
		boost::bind(&GENetWork::HandleAccept_N, this, spConnect, boost::asio::placeholders::error));
}

void GENetWork::HandleAccept_N( GENetConnect::ConnectSharedPtr spConnect, const boost::system::error_code& error )
{
	if (!error)
	{
		GE::Uint32 uSessionID;
		// 这里会改变ConnectMgr内部状态，要注意加锁
		this->m_ConnectMutex.lock();
		if (this->m_ConnectMgr.AddConnect(spConnect, uSessionID))
		{
			// 设置还需的TGW换行数
			spConnect->NeedTGWHeadSize(GENetEnv::Instance()->TGWMaxSize());
			// 开始收发消息
			spConnect->Start();
			// 统计最大连接数
			this->m_uSta_LastSessionID = spConnect->SessionID();
			GE::Uint32 uConnectCnt = this->m_ConnectMgr.ConnectCnt();
			this->m_uSta_MaxConnect = GE_MAX(m_uSta_MaxConnect, uConnectCnt);
		}
		else
		{
			spConnect->Shutdown(enNetConnect_ConnectFull);
			GE_EXC<<"too much network connect("<<m_ConnectMgr.ConnectCnt()<<")."<<GE_END;
		}
		this->m_ConnectMutex.unlock();
	}
	this->AsyncAccept_N();
}

void GENetWork::BoostAsioRun()
{
	this->m_IOS.run();
	GE_OUT<<"BoostAsio is Leave."<<GE_END;
}

void GENetWork::ThreadSendFun()
{
	while(m_bIsRun)
	{
		if (this->m_bThreadSend)
		{
			GE_ITER_UI32(uIdx, this->m_ConnectMgr.MaxConnectCnt())
			{
				if (this->m_ConnectMgr.LockForClearData(uIdx))
				{
					GENetConnect* pConnect = this->m_ConnectMgr.FindConnect(uIdx);
					if (pConnect)
					{
						pConnect->ThreadAsyncSendBlock();
					}
					this->m_ConnectMgr.Unlock(uIdx);
				}
			}
			GEDateTime::Instance()->SleepMsec(100);
		}
		else
		{
			GEDateTime::Instance()->SleepMsec(1000);
		}
	}
}

bool GENetWork::MoveNextMsg()
{
	// 对上次的连接做处理
	if (m_bLastConnectClose)
	{
		this->ClearConnect_L(m_pCurConnect->SessionID());
	}
	this->m_pCurMsg = NULL;
	this->m_bLastConnectClose = false;

	// 对连接管理器的迭代要加锁
	this->m_ConnectMutex.lock();
	GE_ITER_UI32(_idx, m_ConnectMgr.ConnectCnt())
	{
		// 获取下一个连接
		this->m_pCurConnect = this->m_ConnectMgr.NextConnect();
		// 记录迭代次数
		++ m_uIterCnt;
		// 如果该连接是关闭着的，则需要处理这个连接
		if (this->m_pCurConnect->IsShutdown())
		{
			// 一定要在这里保存连接是否是关闭着的
			this->m_bLastConnectClose = true;
			break;
		}
		// 如果该连接中有消息，则需要处理这个连接
		if (this->m_pCurConnect->ReadMsg(&m_pCurMsg))
		{
			// 统计消息条数
			++m_uSta_MsgCnt;
			break;
		}
	}
	this->m_ConnectMutex.unlock();
	// 处理连接计数
	if (m_uIterCnt >= m_ConnectMgr.ConnectCnt())
	{
		m_uIterCnt = 0;
	}
	// 返回结果（要么是连接被关闭了，要么是连接中有消息）
	return m_bLastConnectClose || NULL != this->m_pCurMsg;
}

