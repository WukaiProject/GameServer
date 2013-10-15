/************************************************************************
预处理MySQL
************************************************************************/
#pragma once
#include <string>

#include "GEMySQLdb.h"
#include "GEInteger.h"

class GEMySQLStmt
{
	GE_DISABLE_BOJ_CPY(GEMySQLStmt);
public:
	GEMySQLStmt(MYSQL* pMySQLConnect, const std::string& szSQL);
	~GEMySQLStmt(void);

public:
	void				OnConnect();									//当连接上时
	void				OnDisConnect();									//当断开连接时

	GE::Uint64			Execute();										//执行SQL语句
	GE::Uint64			RealQuery(const std::string& szSQL);			//执行SQL语句
	bool				MoveNextRow();									//移动到下一行
	GE::Uint64			AutoInsertID();									//获取自增长ID
	void				FreeResult();									//释放资源
	GE::Uint64			TotalRowNum();									//总共的行数

	void				ReBindParam();									//重新绑定参数
	void				BindParamI8(GE::Int8& i8);						//绑定参数
	void				BindParamUI8(GE::Uint8& ui8);					//绑定参数
	void				BindParamI16(GE::Int16& i16);					//绑定参数
	void				BindParamUI16(GE::Uint16& ui16);				//绑定参数
	void				BindParamI32(GE::Int32& i32);					//绑定参数
	void				BindParamUI32(GE::Uint32& ui32);				//绑定参数
	void				BindParamI64(GE::Int64& i64);					//绑定参数
	void				BindParamUI64(GE::Uint64& ui64);				//绑定参数
	void				BindParamB4(GE::B4& b4);						//绑定参数
	void				BindParamB8(GE::B8& b8);						//绑定参数
	void				BindParamString(void* pHead, GE::Uint16 uSize);	//绑定参数
	void				BindParamBinary(void* pHead, GE::Uint16 uSzie);	//绑定参数

	void				ReBindResult();									//重新绑定结果
	void				BindResultI8(GE::Int8& i8);						//绑定结果
	void				BindResultUI8(GE::Uint8& ui8);					//绑定结果
	void				BindResultI16(GE::Int16& i16);					//绑定结果
	void				BindResultUI16(GE::Uint16& ui16);				//绑定结果
	void				BindResultI32(GE::Int32& i32);					//绑定结果
	void				BindResultUI32(GE::Uint32& ui32);				//绑定结果
	void				BindResultI64(GE::Int64& i64);					//绑定结果
	void				BindResultUI64(GE::Uint64& ui64);				//绑定结果
	void				BindResultB4(GE::B4& b4);						//绑定结果
	void				BindResultB8(GE::B8& b8);						//绑定结果
	void				BindResultString(void* pHead, GE::Uint16 uSize, unsigned long& uRealSize);		//绑定结果
	void				BindResultBinary(void* pHead, GE::Uint16 uSize, unsigned long& uRealSize);		//绑定结果

private:
	void				ToNextParam();
	void				ToNextResult();

private:
	MYSQL*				m_pMySQLConnect;								//MySQL连接对象
	std::string			m_szSQL;										//SQL预处理语句

	MYSQL_STMT*			m_pMysqlStmt;									//MySQLClient预处理结构体
	MYSQL_BIND*			m_pBindParams;									//MySQLClient绑定参数结构体
	MYSQL_BIND*			m_pBindResults;									//MySQLClient绑定结果结构体

	GE::Uint32			m_uiParamCnt;									//需要的结果个数
	GE::Uint32			m_uiResultCnt;									//需要的参数个数
	GE::Uint32			m_uiCurrentParamCnt;							//绑定的参数个数
	GE::Uint32			m_uiCurrentResultCnt;							//绑定的结果个数
};

