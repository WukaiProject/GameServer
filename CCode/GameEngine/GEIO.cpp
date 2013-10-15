/*系统日志相关*/
#include "GEIO.h"

void RedirectOutAndErrStream( const char* sFilePath )
{
	static std::ofstream _out;
	if (_out.is_open())
	{
		_out.close();
	}
	_out.open(sFilePath, std::ios::app);
	if (!_out)
	{
		GE_EXC<<"file("<<sFilePath<<") error on redirect cout!"<<GE_END;
	}
	else
	{
		std::cout.rdbuf(_out.rdbuf());
		std::cerr.rdbuf(_out.rdbuf());
	}
}

