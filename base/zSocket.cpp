#include "zSocket.h"
#include "Config.h"

zSocket::zSocket(asio::io_service& service )
:socket_(service),strand_(service),check_timer_(service) 
{
	m_bQuit = false;
	m_pBasicTask = NULL;
	bufflen_ = 0;
	m_bQuit = false;
}

void zSocket::Start()
{
	BeginReadNewMessage();

	//开始计时，处理请求
	check_timer_.expires_from_now(boost::posix_time::seconds(CHECK_TASK_INTER));
	//check_timer_.async_wait(strand_.wrap(boost::bind(&zSocket::Check,shared_from_this(),asio::placeholders::error)));
	check_timer_.async_wait(strand_.wrap(boost::bind(&zSocket::Check,this,asio::placeholders::error)));
}

void zSocket::BeginReadNewMessage()
{
	ResetRecvBuffer();
	asio::async_read(socket_,
		asio::buffer(recvBuffer_,PH_LEN),
		strand_.wrap(boost::bind(
		//&zSocket::HandleReadHead, shared_from_this(),
		&zSocket::HandleReadHead, this,
		asio::placeholders::error)));
}

void zSocket::HandleReadHead(const system::error_code& error)
{
	if (!error  && ValidateMsgLen())
	{
		ztLoggerWrite(ZONE,e_Debug,"HandleReadHead:msgLen=%d",GetBodyLen());
		asio::async_read(socket_,
			asio::buffer(GetRecvBodyBuf(), GetBodyLen()),
			//strand_.wrap(boost::bind(&zSocket::HandleReadBody, shared_from_this(),
			strand_.wrap(boost::bind(&zSocket::HandleReadBody, this,
			asio::placeholders::error)));
	}
	else
	{
		SetQuitFlag();
	}

	return;
}

void zSocket::Check(const system::error_code& e)
{
	if (!e)
	{	
		if (m_bQuit)
		{
			Close();
			m_pBasicTask->OnQuit();
			return;
		}

		m_pBasicTask->OnCheck();

		//开始计时，处理请求
		check_timer_.expires_from_now(boost::posix_time::seconds(CHECK_TASK_INTER));
		//check_timer_.async_wait(strand_.wrap(boost::bind(&zSocket::Check,shared_from_this(),asio::placeholders::error)));
		check_timer_.async_wait(strand_.wrap(boost::bind(&zSocket::Check,this,asio::placeholders::error)));
	}
	else      // 会走到这里么
	{
		SetQuitFlag();
		//Close();
		//m_pBasicTask->OnQuit();
	}
}

void zSocket::HandleReadBody(const system::error_code& error)
{
	if (!error)
	{
		//Cmd::t_NullCmd *ptNullCmd = (Cmd::t_NullCmd *)GetRecvBodyBuf();
		char *pBody =(char*) GetRecvBodyBuf();
		//printf("HandleReadBody:cmd=%d,para=%d\n",ptNullCmd->cmd,ptNullCmd->para);
		//msgParse(pBody,GetBodyLen());
		m_pBasicTask->msgParse(pBody,GetBodyLen());
		BeginReadNewMessage();
	}	
	else
	{
		SetQuitFlag();
	}


	return;
}

void zSocket::HandleWrite(const system::error_code& error)
{
	wstate = WS_Complete;
	if(!error)
	{
		m_sendLock.lock();
		if (!sendQueue.empty())
		{			
			t_BufferCmd* ptCmd = sendQueue.front();
			asio::async_write(socket_,
				asio::buffer(&ptCmd->pstrCmd[ptCmd->offset],
				ptCmd->nCmdLen - ptCmd->offset),
				//strand_.wrap(boost::bind(&zSocket::HandleWrite, shared_from_this(),
				strand_.wrap(boost::bind(&zSocket::HandleWrite, this,
				asio::placeholders::error)));
			wstate = WS_On;
			sendQueue.pop();
			SAFE_DELETE(ptCmd);   // 可以delete么			
		}
		m_sendLock.unlock();
	}
	else
	{
		SetQuitFlag();
	}

}


