/************************************************************************
网络连接管理
************************************************************************/
#pragma once
#include <vector>
#include <boost/unordered_map.hpp>
#include "GENetConnect.h"
#include "GEIndex.h"

class GENetConnectMgr
{
	GE_DISABLE_BOJ_CPY(GENetConnectMgr);
	typedef GENetConnect*														ConnectPtr;
	typedef boost::unordered_map<GE::Uint32, GENetConnect::ConnectSharedPtr>	HoldMap;
	typedef std::vector<boost::mutex*>											MutexVector;
public:
	GENetConnectMgr(GE::Uint32 uMaxConnect);
	~GENetConnectMgr(void);

public:
	bool				AddConnect(GENetConnect::ConnectSharedPtr& spConnect, GE::Uint32& uID);	//增加一个连接
	bool				DelConnect(GE::Uint32 uID);												//删除一个连接
	bool				HasConnect(GE::Uint32 uID);												//是否有某个连接
	GENetConnect*		FindConnect(GE::Uint32 uID);											//查找一个连接
	GENetConnect*		FindConnectForHasData(GE::Uint32 uID);									//查找一个连接（为了发送数据）
	GENetConnect*		NextConnect();															//获取下一个连接（内部循环迭代，增删连接迭代依然有效）
	GE::Uint32			ConnectCnt() {return m_IndexMgr.Size();}								//连接的个数
	GE::Uint32			MaxConnectCnt() {return m_IndexMgr.MaxSize();}							//最大连接数
	void				Lock(GE::Uint32 uID);													//锁住某个连接
	bool				LockForClearData(GE::Uint32 uID);										//锁住某个连接（为了清理数据）
	void				Unlock(GE::Uint32 uID);													//释放某个连接

private:
	GEIndex				m_IndexMgr;			//索引ID分配器
	ConnectPtr*			m_pDataArr;			//连接数组
	bool*				m_pHasData;			//是否有未发送的数据
	HoldMap				m_pDataMap;			//用来hold智能指针的map
	MutexVector			m_pMutexVector;		//用来管理每个连接的锁（多锁，减少碰撞）
};

