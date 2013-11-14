// main.cpp 服务器主程序入口
//

#include "stdafx.h"
#include <Windows.h>
#include <libs/system/src/error_code.cpp>
#include "GameGrailCommon.h"
#include "DiceGameServer.h"
#include "GMCommand.h"
CardEntity* cardList[150];
int _tmain(int argc, _TCHAR* argv[])
{
	// 初始化gm指令
	initialize_gm_command();

	DiceGameServer::newInstance();
	if(!DiceGameServer::getInstance().serverInit())
	{
		printf("GameServer init fail...\n");
		return -1;
	}
 
	ztLoggerWrite(ZONE, e_Information, "GameServer Start ");
	DiceGameServer::getInstance().main();
	DiceGameServer::delInstance();
	return 0;
}



