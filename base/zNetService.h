/**
 *
 * 这个类比较通用一点，再创建比较一般的网络服务器程序的时候是比较有用
 * 
 */

#ifndef _zNetService_h_
#define _zNetService_h_

#include <iostream>
#include <string>
 

#include "zService.h"
#include "zTCPServer.h"
 

/**
 * \brief 网络服务器类
 *
 * 实现了网络服务器框架代码，这个类比较通用一点
 *
 */
class zNetService : public zService
{

	public:

		/**
		 * \brief 虚析构函数
		 *
		 */
		virtual ~zNetService() { instance = NULL; };

		/**
		 * \brief 根据得到的TCP/IP连接获取一个连接任务
		 *
		 * \param sock TCP/IP套接口
		 * \param addr 地址
		 */
		virtual zTCPTask* newTCPTask(const uint16_t usPort) = 0;

		/**
		 * \brief 获取连接池中的连接数
		 *
		 * \return 连接数
		 */
		virtual const int getPoolSize() const
		{
			return 0;
		}

		/**
		 * \brief 获取连接池状态
		 *
		 * \return 连接池状态
		 */
		virtual const int getPoolState() const
		{
			return 0;
		}

	protected:

		/**
		 * \brief 构造函数
		 * 
		 * 受保护的构造函数，实现了Singleton设计模式，保证了一个进程中只有一个类实例
		 *
		 * \param name 名称
		 */
		zNetService(const std::string &name) : zService(name)
		{
			instance = this;

			serviceName = name;
			tcpServer = NULL;
		}

		bool init(unsigned short port);
		virtual bool serviceCallback();
		void final();

		asio::io_service& getIOSerivce()
		{
			return tcpServer->getIOService();
		}

	private:

		static zNetService *instance;		/**< 类的唯一实例指针，包括派生类，初始化为空指针 */
		std::string serviceName;			/**< 网络服务器名称 */
		zTCPServer *tcpServer;				/**< TCP服务器实例指针 */

};

#endif

