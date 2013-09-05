#ifndef __zTCPServer__h__
#define __zTCPServer__h__

#include "zLogger.h"
#include "Config.h"
#include "zTCPTask.h"
#include "boost/thread.hpp"

/// A pool of io_service objects.   
class IOServicePool
	: private boost::noncopyable   
{   
public:   
	/// Construct the io_service pool.   
	explicit IOServicePool(std::size_t pool_size = 4) : next_io_service_(0)   
	{   
		if (pool_size == 0)   
			throw std::runtime_error("io_service_pool size is 0");   

		// Give all the io_services work to do so that their run() functions will not   
		// exit until they are explicitly stopped.   
		for (std::size_t i = 0; i < pool_size; ++i)   
		{   
			io_service_ptr io_service(new asio::io_service);   
			work_ptr work(new asio::io_service::work(*io_service));   
			io_services_.push_back(io_service);   
			work_.push_back(work);   
		}   
	}   

	// Run all io_service objects in the pool.   
	void run()   
	{   
		// Create a pool of threads to run all of the io_services.   
		std::vector<boost::shared_ptr<boost::thread> > threads;   
		for (std::size_t i = 0; i < io_services_.size(); ++i)   
		{   
			boost::shared_ptr<boost::thread> thread(new boost::thread(   
				boost::bind(&asio::io_service::run, io_services_[i])));   
			threads.push_back(thread);   
		}   

		// Wait for all threads in the pool to exit.   
		for (std::size_t i = 0; i < threads.size(); ++i)   
			threads[i]->join();   
	}   

	// Stop all io_service objects in the pool.   
	void stop()   
	{   
		// Explicitly stop all io_services.   
		for (std::size_t i = 0; i < io_services_.size(); ++i)   
			io_services_[i]->stop();   
	}   

	// Get an io_service to use.   
	asio::io_service& get_io_service()   
	{   
		// Use a round-robin scheme to choose the next io_service to use.   
		asio::io_service& io_service = *io_services_[next_io_service_];   
		++next_io_service_;   
		if (next_io_service_ == io_services_.size())   
			next_io_service_ = 0;   
		return io_service;   
	}   

	asio::io_service& get_first_service()
	{
		return *io_services_[0];
	}

private:   
	typedef boost::shared_ptr<asio::io_service> io_service_ptr;   
	typedef boost::shared_ptr<asio::io_service::work> work_ptr;   

	/// The pool of io_services.   
	std::vector<io_service_ptr> io_services_;   

	/// The work that keeps the io_services running.   
	std::vector<work_ptr> work_;   

	/// The next io_service to use for a connection.   
	std::size_t next_io_service_;   
};   

class zNetService;

class zTCPServer : private boost::noncopyable
{
public:

	virtual bool init();

	bool run();

	void stop();

	void exit();

	bool bind(const char *pstrIP, uint16_t usPort, zNetService* pService);

public:
	virtual zTCPTask* CreateTask(uint16_t usPort) ;

	//void handle_accept(zTCPTask_Ptr pSession,const asio::error_code& error)
	void handle_accept(zTCPTask* pSession,zNetService* pService,const system::error_code& error);

	void AddTask(zTCPTask* pTask)
	{
		zMutex_scope_lock scope_lock(m_lock);
		m_TaskVec.push_back(pTask);
	}

	virtual void Check()   
	{
		if (time(NULL) - m_checktime > 60)
		{
			// TODO : need for test and debug
			ztLoggerWrite(ZONE,e_Debug,"Main::Check session size:%d",m_TaskVec.size());
			m_checktime = time(NULL);

			zMutex_scope_lock scope_lock(m_lock);
			for(std::vector<zTCPTask*>::iterator iter = m_TaskVec.begin(); iter != m_TaskVec.end();)
			{
				zTCPTask* pTask = *iter;
				if(pTask->GetState() == zTCPTask::TaskState_recycle)
				{
					ztLoggerWrite(ZONE,e_Debug,"Recycle Task:%d", pTask);
					iter = m_TaskVec.erase(iter++);

					SAFE_DELETE(pTask);
				}
				else
				{
					++iter;
				}
			}
		}
	}
	
public:

//protected:
	zTCPServer(){m_bMain = true;m_checktime = time(NULL);}

	virtual ~zTCPServer();

	asio::io_service& getIOService()
	{
		return m_ioServicePool_.get_io_service();
	}

	IOServicePool& getServicePool()
	{
		return m_ioServicePool_;
	}
protected:
	//asio::io_service m_ioService_;
	asio::ip::tcp::acceptor *m_pAcceptor_;
	
	boost::shared_ptr<boost::thread> m_ioThread_;   
	boost::shared_ptr<boost::thread> m_workThread_; 

	IOServicePool m_ioServicePool_;
	IOServicePool m_ioServiceWorkPool_; 

	bool m_bMain;
	std::string m_strIP;
	uint16_t	m_usPort;

	//session vector
	std::vector<zTCPTask*> m_TaskVec;    //由于Task是提前加入的,所以总是有一个zTCPTask在里面
	time_t		m_checktime;
	zMutex		m_lock;

	//asio::deadline_timer *m_pCheckTimer;   //检测session信息
};

#endif  //__zTCPServer__h__
