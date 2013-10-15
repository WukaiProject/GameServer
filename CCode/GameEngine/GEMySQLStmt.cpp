/*预处理MySQL*/
#include "GEMySQLStmt.h"
#include "GEIO.h"

GEMySQLStmt::GEMySQLStmt( MYSQL* pMySQLConnect, const std::string& szSQL )
	: m_pMySQLConnect(pMySQLConnect)
	, m_szSQL(szSQL)
	, m_pMysqlStmt(NULL), m_pBindParams(NULL), m_pBindResults(NULL)
	, m_uiParamCnt(0), m_uiResultCnt(0), m_uiCurrentParamCnt(0), m_uiCurrentResultCnt(0)
{

}

GEMySQLStmt::~GEMySQLStmt( void )
{
	OnDisConnect();
}

void GEMySQLStmt::OnConnect()
{
	m_pMysqlStmt = mysql_stmt_init(m_pMySQLConnect);
	if(!m_pMysqlStmt)
	{
		// 如果创建失败,直接崩溃
		GE_ERROR(0 == "mysql_stmt_init fail.");
	}

	GE::Uint32 uiSize = static_cast<GE::Uint32>(m_szSQL.size());
	if(mysql_stmt_prepare(m_pMysqlStmt, m_szSQL.c_str(), uiSize))
	{
		GE_EXC<<mysql_error(m_pMySQLConnect)<<GE_END;
		GE_ERROR(0 == "mysql_stmt_prepare");

	}

	// 获取绑定参数个数
	GE::Uint32 uiParamCnt = mysql_stmt_param_count(m_pMysqlStmt);
	// 获取绑定结果个数
	GE::Uint32 uiResultCnt = 0;
	MYSQL_RES* pRes = mysql_stmt_result_metadata(m_pMysqlStmt);
	if(pRes)
	{
		uiResultCnt = mysql_num_fields(pRes);
		mysql_free_result(pRes);
	}
	// 绑定参数结构体
	if(uiParamCnt)
	{
		m_pBindParams = new MYSQL_BIND[uiParamCnt];
		memset(m_pBindParams, 0, uiParamCnt * sizeof(MYSQL_BIND));
	}
	// 绑定结果结构体
	if(uiResultCnt)
	{
		m_pBindResults = new MYSQL_BIND[uiResultCnt];
		memset(m_pBindResults, 0, uiResultCnt * sizeof(MYSQL_BIND));
	}
	// 初始化状态
	m_uiParamCnt = uiParamCnt;
	m_uiResultCnt = uiResultCnt;
	m_uiCurrentParamCnt = 0;
	m_uiCurrentResultCnt = 0;
}

void GEMySQLStmt::OnDisConnect()
{
	if(m_pMysqlStmt)
	{
		mysql_stmt_close(m_pMysqlStmt);
		m_pMysqlStmt = NULL;
	}
	if(m_pBindParams)
	{
		delete [] m_pBindParams;
		m_pBindParams = NULL;
	}
	if(m_pBindResults)
	{
		delete [] m_pBindParams;
		m_pBindParams = NULL;
	}
}

GE::Uint64 GEMySQLStmt::Execute()
{
	// 绑定的参数必须和需要的参数相等
	GE_ERROR(m_uiParamCnt == m_uiCurrentParamCnt);
	// 绑定的结果个数和需要的结果个数相等
	GE_ERROR(m_uiResultCnt == m_uiCurrentResultCnt);

	if(mysql_stmt_execute(m_pMysqlStmt))
	{
		unsigned int error =  mysql_errno(m_pMySQLConnect);
		// 再执行一遍,特殊处理MySQL缓存重新加载的问题。
		if (1615 == error)
		{
			if(mysql_stmt_execute(m_pMysqlStmt))
			{
				GE_EXC<<"MySQL execute Error."<<error<<" "<<mysql_error(m_pMySQLConnect)<<GE_END;
				return -1;
			}
		}
		else
		{
			GE_EXC<<"MySQL execute Error."<<error<<" "<<mysql_error(m_pMySQLConnect)<<GE_END;
			return -1;
		}
	}
	if (mysql_stmt_store_result(m_pMysqlStmt))
	{
		GE_EXC<<"MySQL store result Error."<<mysql_errno(m_pMySQLConnect)<<" "<<mysql_error(m_pMySQLConnect)<<GE_END;
	}
	return mysql_affected_rows(m_pMySQLConnect);
}

GE::Uint64 GEMySQLStmt::RealQuery( const std::string& szSQL )
{
	if (mysql_real_query(m_pMySQLConnect, szSQL.data(), static_cast<unsigned long>(szSQL.length())))
	{
		GE_EXC<<"MySQL execute Error."<<mysql_errno(m_pMySQLConnect)<<" "<<mysql_error(m_pMySQLConnect)<<GE_END;
		return -1;
	}
	return mysql_affected_rows(m_pMySQLConnect);
}

bool GEMySQLStmt::MoveNextRow()
{
	int result = mysql_stmt_fetch(m_pMysqlStmt);
	if (result)
	{
		GE_EXC<<"Error happend On GEMySQLStmt::MoveNextRow() where result = "<<result<<" Error:"<<mysql_errno(m_pMySQLConnect)<<" "<<mysql_error(m_pMySQLConnect)<<GE_END;
	}
	return  (0 == result);
}

GE::Uint64 GEMySQLStmt::AutoInsertID()
{
	return mysql_insert_id(m_pMySQLConnect);
}

void GEMySQLStmt::ReBindParam()
{
	m_uiCurrentParamCnt = 0;
}

void GEMySQLStmt::BindParamI8( GE::Int8& i8 )
{
	MYSQL_BIND& bind = m_pBindParams[m_uiCurrentParamCnt];
	bind.buffer = &i8;
	bind.buffer_type = MYSQL_TYPE_TINY;
	bind.is_null = 0;
	bind.length = 0;
	bind.is_unsigned = false;
	ToNextParam();
}

void GEMySQLStmt::BindParamUI8( GE::Uint8& ui8 )
{
	MYSQL_BIND& bind = m_pBindParams[m_uiCurrentParamCnt];
	bind.buffer = &ui8;
	bind.buffer_type = MYSQL_TYPE_TINY;
	bind.is_null = 0;
	bind.length = 0;
	bind.is_unsigned = true;
	ToNextParam();
}

void GEMySQLStmt::BindParamI16( GE::Int16& i16 )
{
	MYSQL_BIND& bind = m_pBindParams[m_uiCurrentParamCnt];
	bind.buffer = &i16;
	bind.buffer_type = MYSQL_TYPE_SHORT;
	bind.is_null = 0;
	bind.length = 0;
	bind.is_unsigned = false;
	ToNextParam();
}

void GEMySQLStmt::BindParamUI16( GE::Uint16& ui16 )
{
	MYSQL_BIND& bind = m_pBindParams[m_uiCurrentParamCnt];
	bind.buffer = &ui16;
	bind.buffer_type = MYSQL_TYPE_SHORT;
	bind.is_null = 0;
	bind.length = 0;
	bind.is_unsigned = true;
	ToNextParam();
}

void GEMySQLStmt::BindParamI32( GE::Int32& i32 )
{
	MYSQL_BIND& bind = m_pBindParams[m_uiCurrentParamCnt];
	bind.buffer = &i32;
	bind.buffer_type = MYSQL_TYPE_LONG;
	bind.is_null = 0;
	bind.length = 0;
	bind.is_unsigned = false;
	ToNextParam();
}

void GEMySQLStmt::BindParamUI32( GE::Uint32& ui32 )
{
	MYSQL_BIND& bind = m_pBindParams[m_uiCurrentParamCnt];
	bind.buffer = &ui32;
	bind.buffer_type = MYSQL_TYPE_LONG;
	bind.is_null = 0;
	bind.length = 0;
	bind.is_unsigned = true;
	ToNextParam();
}

void GEMySQLStmt::BindParamI64( GE::Int64& i64 )
{
	MYSQL_BIND& bind = m_pBindParams[m_uiCurrentParamCnt];
	bind.buffer = &i64;
	bind.buffer_type = MYSQL_TYPE_LONGLONG;
	bind.is_null = 0;
	bind.length = 0;
	bind.is_unsigned = false;
	ToNextParam();
}

void GEMySQLStmt::BindParamUI64( GE::Uint64& ui64 )
{
	MYSQL_BIND& bind = m_pBindParams[m_uiCurrentParamCnt];
	bind.buffer = &ui64;
	bind.buffer_type = MYSQL_TYPE_LONGLONG;
	bind.is_null = 0;
	bind.length = 0;
	bind.is_unsigned = true;
	ToNextParam();
}

void GEMySQLStmt::BindParamB4( GE::B4& b4 )
{
	MYSQL_BIND& bind = m_pBindParams[m_uiCurrentParamCnt];
	bind.buffer = &b4;
	bind.buffer_type = MYSQL_TYPE_LONG;
	bind.is_null = 0;
	bind.length = 0;
	bind.is_unsigned = true;
	ToNextParam();
}

void GEMySQLStmt::BindParamB8( GE::B8& b8 )
{
	MYSQL_BIND& bind = m_pBindParams[m_uiCurrentParamCnt];
	bind.buffer = &b8;
	bind.buffer_type = MYSQL_TYPE_LONGLONG;
	bind.is_null = 0;
	bind.length = 0;
	bind.is_unsigned = true;
	ToNextParam();
}

void GEMySQLStmt::BindParamString( void* pHead, GE::Uint16 uSize )
{
	MYSQL_BIND& bind = m_pBindParams[m_uiCurrentParamCnt];
	bind.buffer = pHead;
	bind.buffer_type = MYSQL_TYPE_VAR_STRING;
	bind.buffer_length = uSize;
	bind.is_null = 0;
	bind.length_value = uSize;
	bind.length = &bind.length_value;
	ToNextParam();
}

void GEMySQLStmt::BindParamBinary( void* pHead, GE::Uint16 uSize )
{
	MYSQL_BIND& bind = m_pBindParams[m_uiCurrentParamCnt];
	bind.buffer = pHead;
	bind.buffer_type = MYSQL_TYPE_BLOB;
	bind.buffer_length = uSize;
	bind.is_null = 0;
	bind.length_value = uSize;
	bind.length = &bind.length_value;
	ToNextParam();
}

void GEMySQLStmt::ReBindResult()
{
	m_uiCurrentResultCnt = 0;
}

void GEMySQLStmt::BindResultI8( GE::Int8& i8 )
{
	MYSQL_BIND& bind = m_pBindResults[m_uiCurrentResultCnt];
	bind.buffer = &i8;
	bind.buffer_type = MYSQL_TYPE_TINY;
	bind.is_null = 0;
	bind.length = 0;
	bind.is_unsigned = false;
	ToNextResult();
}

void GEMySQLStmt::BindResultUI8( GE::Uint8& ui8 )
{
	MYSQL_BIND& bind = m_pBindResults[m_uiCurrentResultCnt];
	bind.buffer = &ui8;
	bind.buffer_type = MYSQL_TYPE_TINY;
	bind.is_null = 0;
	bind.length = 0;
	bind.is_unsigned = true;
	ToNextResult();
}

void GEMySQLStmt::BindResultI16( GE::Int16& i16 )
{
	MYSQL_BIND& bind = m_pBindResults[m_uiCurrentResultCnt];
	bind.buffer = &i16;
	bind.buffer_type = MYSQL_TYPE_SHORT;
	bind.is_null = 0;
	bind.length = 0;
	bind.is_unsigned = false;
	ToNextResult();
}

void GEMySQLStmt::BindResultUI16( GE::Uint16& ui16 )
{
	MYSQL_BIND& bind = m_pBindResults[m_uiCurrentResultCnt];
	bind.buffer = &ui16;
	bind.buffer_type = MYSQL_TYPE_SHORT;
	bind.is_null = 0;
	bind.length = 0;
	bind.is_unsigned = true;
	ToNextResult();
}

void GEMySQLStmt::BindResultI32( GE::Int32& i32 )
{
	MYSQL_BIND& bind = m_pBindResults[m_uiCurrentResultCnt];
	bind.buffer = &i32;
	bind.buffer_type = MYSQL_TYPE_LONG;
	bind.is_null = 0;
	bind.length = 0;
	bind.is_unsigned = false;
	ToNextResult();
}

void GEMySQLStmt::BindResultUI32( GE::Uint32& ui32 )
{
	MYSQL_BIND& bind = m_pBindResults[m_uiCurrentResultCnt];
	bind.buffer = &ui32;
	bind.buffer_type = MYSQL_TYPE_LONG;
	bind.is_null = 0;
	bind.length = 0;
	bind.is_unsigned = true;
	ToNextResult();
}

void GEMySQLStmt::BindResultI64( GE::Int64& i64 )
{
	MYSQL_BIND& bind = m_pBindResults[m_uiCurrentResultCnt];
	bind.buffer = &i64;
	bind.buffer_type = MYSQL_TYPE_LONGLONG;
	bind.is_null = 0;
	bind.length = 0;
	bind.is_unsigned = false;
	ToNextResult();
}

void GEMySQLStmt::BindResultUI64( GE::Uint64& ui64 )
{
	MYSQL_BIND& bind = m_pBindResults[m_uiCurrentResultCnt];
	bind.buffer = &ui64;
	bind.buffer_type = MYSQL_TYPE_LONGLONG;
	bind.is_null = 0;
	bind.length = 0;
	bind.is_unsigned = true;
	ToNextResult();
}

void GEMySQLStmt::BindResultB4( GE::B4& b4 )
{
	MYSQL_BIND& bind = m_pBindResults[m_uiCurrentResultCnt];
	bind.buffer = &b4;
	bind.buffer_type = MYSQL_TYPE_LONG;
	bind.is_null = 0;
	bind.length = 0;
	bind.is_unsigned = true;
	ToNextResult();
}

void GEMySQLStmt::BindResultB8( GE::B8& b8 )
{
	MYSQL_BIND& bind = m_pBindResults[m_uiCurrentResultCnt];
	bind.buffer	= &b8;
	bind.buffer_type = MYSQL_TYPE_LONGLONG;
	bind.is_null = 0;
	bind.length = 0;
	bind.is_unsigned = true;
	ToNextResult();
}

void GEMySQLStmt::BindResultString( void* pHead, GE::Uint16 uSize, unsigned long& uRealSize )
{
	MYSQL_BIND& bind = m_pBindResults[m_uiCurrentResultCnt];
	bind.buffer_type = MYSQL_TYPE_VAR_STRING;
	bind.buffer = pHead;
	bind.buffer_length = uSize;
	bind.is_null = 0;
	bind.length = &uRealSize;
	ToNextResult();
}

void GEMySQLStmt::BindResultBinary( void* pHead, GE::Uint16 uSize, unsigned long& uRealSize )
{
	MYSQL_BIND& bind = m_pBindResults[m_uiCurrentResultCnt];
	bind.buffer_type = MYSQL_TYPE_BLOB;
	bind.buffer = pHead;
	bind.buffer_length = uSize;
	bind.is_null = 0;
	bind.length = &uRealSize;
	ToNextResult();
}

void GEMySQLStmt::ToNextParam()
{
	++m_uiCurrentParamCnt;
	if (m_uiCurrentParamCnt == m_uiParamCnt)
	{
		GE_ERROR(0 == mysql_stmt_bind_param(m_pMysqlStmt, m_pBindParams));
	}
	GE_ERROR(m_uiCurrentParamCnt <= m_uiParamCnt);
}

void GEMySQLStmt::ToNextResult()
{
	++m_uiCurrentResultCnt;
	if (m_uiCurrentResultCnt == m_uiResultCnt)
	{
		GE_ERROR(0 == mysql_stmt_bind_result(m_pMysqlStmt,m_pBindResults));
	}
	GE_ERROR(m_uiCurrentResultCnt <= m_uiResultCnt);
}

void GEMySQLStmt::FreeResult()
{
	mysql_stmt_free_result(m_pMysqlStmt);
}

GE::Uint64 GEMySQLStmt::TotalRowNum()
{
	return mysql_stmt_num_rows(m_pMysqlStmt);
}

