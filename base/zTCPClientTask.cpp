#include "zTCPClientTask.h"
#include "zLogger.h"

zTCPClientTask::zTCPClientTask(asio::io_service& service,const char* ip, const uint16_t port):m_Service(service)
{
	m_pSocket = new zSocket(m_Service);
	m_strIP = ip;
	m_usPort = port;
}


bool zTCPClientTask::connect()
{
	m_pSocket->SetProcessor(this);
	asio::ip::tcp::endpoint epoint(asio::ip::address::from_string(m_strIP),m_usPort);
	m_pSocket->socket().async_connect(epoint, boost::bind(&zTCPClientTask::handle_connect,this,asio::placeholders::error));
	ztLoggerWrite(ZONE,e_Debug,"begin,ready to connect:(%s:%d) ",m_strIP.c_str(),m_usPort);
	return true;
}

void zTCPClientTask::handle_connect(const system::error_code& error)
{
	if (!error)
	{
		ztLoggerWrite(ZONE,e_Debug,"handle_connect successful ");
		OnConnect();
		m_pSocket->Start();
	}
	else
	{
		ztLoggerWrite(ZONE,e_Debug, "handle_connect error :(%d,%s) ", error.value(), error.message());
		asio::ip::tcp::endpoint epoint(asio::ip::address::from_string(m_strIP), m_usPort);
		m_pSocket->socket().async_connect(epoint, boost::bind(&zTCPClientTask::handle_connect, this, asio::placeholders::error));
		ztLoggerWrite(ZONE,e_Debug, "error,ready to connect:(%s:%d) ", m_strIP.c_str(), m_usPort);
	}
}

void zTCPClientTask::OnConnect()
{

}

bool zTCPClientTask::msgParse(const void *pstrMsg, const uint32_t)
{
	return true;
}

void zTCPClientTask::OnQuit()
{
	ztLoggerWrite(ZONE,e_Debug,"zTCPClientTask::OnQuit ");
	SAFE_DELETE(m_pSocket);
	m_pSocket = new zSocket(m_Service);
	connect();
	return;
}

void zTCPClientTask::OnCheck()
{
	//ztLoggerWrite(ZONE,e_Debug,"zTCPClientTask::OnCheck ");
	return;
}
