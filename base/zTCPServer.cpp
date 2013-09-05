#include "zTCPServer.h"
#include "Config.h"
#include "zNetService.h"

bool zTCPServer::init()
{
	m_ioThread_.reset(new boost::thread(boost::bind(&IOServicePool::run  
		, &m_ioServicePool_))); 
	return true;
}

bool zTCPServer::run()
{
	//m_ioThread_.reset(new asio::thread(boost::bind(&IOServicePool::run  
		//, &m_ioServicePool_)));   
	//m_workThread_.reset(new boost::thread(boost::bind(&IOServicePool::run  
	// , &m_ioServiceWorkPool_))); 

	while (m_bMain)
	{
		//todo 可加入一些统计功能
		//UserSessionManager::getInstance().doCmd();
		Check();
		SLEEP(0);
	}

	return true;
}

void zTCPServer::stop()   
{   
	m_ioServicePool_.stop();   
	//m_ioServiceWorkPool_.stop();   

	m_ioThread_->join();   
	//m_workThread_->join();   
	ztLoggerWrite(ZONE,e_Debug,"zTCPServer::stop!");
}  

void zTCPServer::exit()
{
	m_bMain = false;
}

zTCPServer::~zTCPServer()
{
	//m_ioService_.stop();
	stop();
	m_bMain = false;
	SAFE_DELETE(m_pAcceptor_);

	zMutex_scope_lock scope_lock(m_lock);
	for(std::vector<zTCPTask*>::iterator iter = m_TaskVec.begin(); iter != m_TaskVec.end(); ++iter)
	{
		zTCPTask* pTask = *iter;
		SAFE_DELETE(pTask);
	}
	m_TaskVec.clear();
}


bool zTCPServer::bind(const char *pstrIP, uint16_t usPort, zNetService* pService)
{	
	//m_pAcceptor_ = new asio::ip::tcp::acceptor(m_ioServicePool_.get_io_service());
	m_pAcceptor_ = new asio::ip::tcp::acceptor(m_ioServicePool_.get_first_service());
	asio::ip::tcp::endpoint epoint(asio::ip::address::from_string(pstrIP),usPort);
	m_pAcceptor_->open(epoint.protocol());
	m_pAcceptor_->set_option(asio::ip::tcp::acceptor::reuse_address(true));
	m_pAcceptor_->bind(epoint);

	m_pAcceptor_->listen();

	//m_pCheckTimer = new asio::deadline_timer(m_ioServicePool_.get_first_service());
	//m_pCheckTimer.expires_from_now(boost::posix_time::seconds(CHECK_TASK_INTER));
	//m_pCheckTimer.async_wait(boost::bind(&zSocket::Check,this,asio::placeholders::error));

	m_usPort = usPort;
	m_strIP = pstrIP;
	//zTCPTask_Ptr pNewSession(new zTCPTask(m_ioServicePool_.get_io_service()));
	//zTCPTask_Ptr pNewSession(CreateTask(usPort));
	for (int i = 0; i < 10; ++i)  //投递10个
	{
		//zTCPTask * pNewSession = CreateTask(usPort);
		zTCPTask * pNewSession = pService->newTCPTask(m_usPort);
		m_pAcceptor_->async_accept(pNewSession->socket(),
			boost::bind(&zTCPServer::handle_accept,this,pNewSession,pService,asio::placeholders::error));
		AddTask(pNewSession);
	}

	return true;
}

zTCPTask* zTCPServer::CreateTask(uint16_t usPort) 
{
	return new zTCPTask(m_ioServicePool_.get_io_service());
}

//void handle_accept(zTCPTask_Ptr pSession,const asio::error_code& error)
void zTCPServer::handle_accept(zTCPTask* pSession,zNetService* pService,const system::error_code& error)
{
	ztLoggerWrite(ZONE,e_Debug,"handle_accept ");
	if (!error)
	{
		pSession->Start();
		//zTCPTask_Ptr pNewSession(CreateTask(m_usPort));
		//zTCPTask_Ptr pNewSession(new zTCPTask(m_ioServicePool_.get_io_service()));
		//zTCPTask * pNewSession = CreateTask(usPort);
		zTCPTask * pNewSession = pService->newTCPTask(m_usPort);
		m_pAcceptor_->async_accept(pNewSession->socket(),
			boost::bind(&zTCPServer::handle_accept,this,pNewSession,pService,asio::placeholders::error));

		AddTask(pNewSession);
	}
}

