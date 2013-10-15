/************************************************************************
网络层相关定义
************************************************************************/
#pragma once
#include "GEInteger.h"
#include "GENetMessage.h"

// 连接的默认参数
#define DEFUAL_SEND_BLOCK_SIZE		MAX_UINT16			//发送缓冲区消息块大小
#define DEFUAL_SEND_BLOCK_NUM		1000				//发送缓冲区消息块个数
#define DEFUAL_RECV_BLOCK_SIZE		MAX_UINT16			//接收缓冲区消息块大小
#define DEFUAL_RECV_BLOCK_NUM		1000				//接收缓冲区消息块个数

struct ConnectParam
{
	ConnectParam()
	{
		uSendBlockSize = DEFUAL_SEND_BLOCK_SIZE;
		uSendBlockNum = DEFUAL_SEND_BLOCK_NUM;
		uRecvBlockSize = DEFUAL_RECV_BLOCK_SIZE;
		uRecvBlockNum = DEFUAL_RECV_BLOCK_NUM;
	}
	GE::Uint16		uSendBlockSize;
	GE::Uint16		uSendBlockNum;
	GE::Uint16		uRecvBlockSize;
	GE::Uint16		uRecvBlockNum;
};

// 网络消息重定向字节
#define MSG_FORWARD_SIZE			8

#pragma pack(push)
#pragma pack(4)

// 引擎层定义的消息
enum GEMsgType
{
	enGEMsg_None,						//无效的消息
	enGEMsg_Ping,						//心跳包
	enGEMsg_Forward_From,				//多合1，表明消息从哪儿来（用于网关重定向）
	enGEMsg_Forward_To,					//1拆多，表明消息要到哪儿去（用于网关重定向）
	enGEMsg_Forward_Other,				//1拆多，表明消息要到其他的连接中去




	enGEMsg_AutoEnd,					//引擎层的最后一个消息类型
	enGEMsg_End = 255,					//引擎层的最后一个消息类型
};

MSG_BEGIN(MsgForwardFrom, enGEMsg_Forward_From)
	GE::Uint32 uSessionID;
};

MSG_BEGIN(MsgForwardTo, enGEMsg_Forward_To)
	GE::Uint32 uSessionID;
};

MSG_BEGIN(MsgForwardOther, enGEMsg_Forward_Other)
	GE::Uint32 uWho;
};

GE_STATIC_ASSERT(MSG_FORWARD_SIZE == sizeof(MsgForwardFrom));
GE_STATIC_ASSERT(MSG_FORWARD_SIZE == sizeof(MsgForwardTo));
GE_STATIC_ASSERT(MSG_FORWARD_SIZE == sizeof(MsgForwardOther));
GE_STATIC_ASSERT(enGEMsg_AutoEnd < enGEMsg_End);

#pragma pack(pop)

