/**
 * \brief 定义服务器框架类<code>zService</code>
 */

#ifndef _zService_h_
#define _zService_h_

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
 
#include <assert.h>
#include "boost/noncopyable.hpp"

/**
 * \brief 定义了服务器的框架基类
 *
 * 所有服务器程序实体需要继承这个类，并且不管是其有多少个子类，整个运行环境只有一个类的实例<br>
 * 只要派生类使用Singleton设计模式实现就可以了
 *
 */
class zService : private boost::noncopyable
{

	public:

		/**
		 * \brief 虚析构函数
		 *
		 */
		virtual ~zService() { serviceInst = NULL; };

		/**
		 * \brief 重新读取配置文件，为HUP信号的处理函数
		 *
		 * 缺省什么事情都不干，只是简单输出一个调试信息，重载这个函数干想干的事情
		 *
		 */
		virtual void reloadConfig()
		{
 
		}

		/**
		 * \brief 判断主循环是否结束
		 *
		 * 如果返回true，将结束主回调
		 *
		 * \return 主循环是否结束
		 */
		bool isTerminate() const
		{
			return terminate;
		}

		/**
		 * \brief 结束主循环，也就是结束主回调函数
		 *
		 */
		void Terminate()
		{
			terminate = true;
		}

		void main();

		/**
		 * \brief 返回服务的实例指针
		 *
		 * \return 服务的实例指针
		 */
		static zService *serviceInstance()
		{
			return serviceInst;
		}

 

	protected:

		/**
		 * \brief 构造函数
		 *
		 */
		zService(const std::string &name) : name(name)
		{
			serviceInst = this;

			terminate = false;
		}

/**
 * \brief 在最终实现的子类中必须重载
 *
 */
		virtual bool init();

		/**
		 * \brief 确认服务器初始化成功，即将进入主回调函数
		 *
		 * \return 确认是否成功
		 */
		virtual bool validate()
		{
			return true;
		}

		/**
		 * \brief 服务程序的主回调函数，主要用于监听服务端口，如果返回false将结束程序，返回true继续执行服务
		 *
		 * \return 回调是否成功
		 */
		virtual bool serviceCallback() = 0;

		/**
		 * \brief 结束服务器程序，回收资源，纯虚函数，子类需要实现这个函数
		 *
		 */
		virtual void final() = 0;

	private:

		static zService *serviceInst;		/**< 类的唯一实例指针，包括派生类，初始化为空指针 */

		std::string name;					/**< 服务名称 */
		bool terminate;						/**< 服务结束标记 */

};

#endif

