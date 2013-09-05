#ifndef __zTCPClientTask__h__
#define __zTCPClientTask__h__

#include "zType.h"
#include "zSocket.h"
#include <string>
#include <boost/asio.hpp>
class zTCPClientTask : public zProcessor, private boost::noncopyable 
{
public:
	zTCPClientTask(asio::io_service& service,const char *ip, const uint16_t port);

	virtual ~zTCPClientTask(){}

	bool connect();

	bool SendCmd(const void* pstrCmd,const int32_t nCmdLen)
	{	
		return m_pSocket->SendCmd(pstrCmd,nCmdLen);;
	}

	void handle_connect(const system::error_code& error);

	virtual bool msgParse(const void *pstrMsg, const uint32_t);

	virtual void OnQuit();

	virtual void OnCheck();

	virtual void OnConnect();

protected:
//private:
	asio::io_service& m_Service;
	zSocket *m_pSocket;
	std::string m_strIP;
	uint16_t	m_usPort;
};

#endif //__zTCPClientTask__h__
