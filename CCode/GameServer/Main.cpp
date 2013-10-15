/*Main入口*/

//#ifdef WIN
//#define WIN32_LEAN_AND_MEAN
//#include "vld.h"
//#endif
#include "GameServer.h"

int main(int argc, char* argv[])
{
	GameServer::New();
	GameServer::Delete();
	return 0;
}

