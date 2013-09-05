/**
 * \file
 * \version  $Id: zService.cpp 9 2005-06-28 12:03:37Z song $
 * \author  Songsiliang,songsiliang@netease.com
 * \date 2004年11月25日 10时34分12秒 CST
 * \brief 实现服务器框架类
 *
 * 
 */


#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <assert.h>
#include <signal.h>

#include "zService.h"
  
/**
 * \brief CTRL + C等信号的处理函数，结束程序
 *
 * \param signum 信号编号
 */
static void ctrlcHandler(int signum)
{
	//如果没有初始化zService实例，表示出错
	zService *instance = zService::serviceInstance();
	instance->Terminate();
}

/**
 * \brief HUP信号处理函数
 *
 * \param signum 信号编号
 */
static void hupHandler(int signum)
{
	//如果没有初始化zService实例，表示出错
	zService *instance = zService::serviceInstance();
	instance->reloadConfig();
}

zService *zService::serviceInst = NULL;

/**
 * \brief 初始化服务器程序，子类需要实现这个函数
 *
 * \return 是否成功
 */
bool zService::init()
{
 
	return true;
}

/**
 * \brief 服务程序框架的主函数
 */
void zService::main()
{
	//初始化程序，并确认服务器启动成功
	if (init()
			&& validate())
	{
		//运行主回调线程
		while(!isTerminate())
		{
			if (!serviceCallback())
			{
				break;
			}
		}
	}

	//结束程序，释放相应的资源
	final();
}

