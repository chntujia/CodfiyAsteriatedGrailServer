#include "zSocket2.h"
#include "Config.h"


zSocket2::zSocket2(asio::io_service& service )
:socket_(service),strand_(service),check_timer_(service) 
{
	m_bQuit = false;
	m_pBasicTask = NULL;
	bufflen_ = 0;
	m_bQuit = false;
}

void zSocket2::Start()
{
	int len = PACKET_MASK - bufflen_;
	socket_.async_read_some(asio::buffer(&recvBuffer_[bufflen_],len),
		strand_.wrap(boost::bind(
		//&zSocket::HandleReadHead, shared_from_this(),
		&zSocket2::HandleRead, this,
		asio::placeholders::error,asio::placeholders::bytes_transferred)));

	//开始计时，处理请求
	check_timer_.expires_from_now(boost::posix_time::seconds(CHECK_TASK_INTER));
	check_timer_.async_wait(strand_.wrap(boost::bind(&zSocket2::Check,this,asio::placeholders::error)));
}

void zSocket2::HandleRead(const system::error_code& error,std::size_t bytes_transferred)
{
	if (!error )// && ValidateMsgLen())
	{
		//////
		bufflen_ += bytes_transferred;
		//bcopy(recvBuffer_,totalRecvBuff,bufflen_);
		while (bufflen_ >= GetMsgLen())
		{
			m_pBasicTask->msgParse(GetRecvBodyBuf(),GetBodyLen());
			bufflen_ -= GetMsgLen();
			memmove(recvBuffer_,recvBuffer_ + GetMsgLen(),bufflen_);
		}

		////////
		//ResetRecvBuffer();
		int len = PACKET_MASK - bufflen_;
		socket_.async_read_some(asio::buffer(&recvBuffer_[bufflen_],len),
			strand_.wrap(boost::bind(&zSocket2::HandleRead, this,
			asio::placeholders::error,asio::placeholders::bytes_transferred)));
	}
	else
	{
		SetQuitFlag();
	}

	return;
}

void zSocket2::Check(const system::error_code& e)
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
		check_timer_.async_wait(strand_.wrap(boost::bind(&zSocket2::Check,this,asio::placeholders::error)));
	}
	else      // 会走到这里么
	{
		SetQuitFlag();
		//Close();
		//m_pBasicTask->OnQuit();
	}
}

void zSocket2::HandleWrite(const system::error_code& error)
{
	if(!error)
	{
		wstate = WS_Complete;
		//m_sendLock.lock();
		//if (!sendQueue.empty())
		//{			
		//	t_BufferCmd* ptCmd = sendQueue.front();
		//	asio::async_write(socket_,
		//		asio::buffer(&ptCmd->pstrCmd[ptCmd->offset],
		//		ptCmd->nCmdLen - ptCmd->offset),
		//		//strand_.wrap(boost::bind(&zSocket::HandleWrite, shared_from_this(),
		//		strand_.wrap(boost::bind(&zSocket2::HandleWrite, this,
		//		asio::placeholders::error)));
		//	wstate = WS_On;
		//	sendQueue.pop();
		//	SAFE_DELETE(ptCmd);   // 可以delete么
		//}
		//m_sendLock.unlock();
	}
	else
	{
		SetQuitFlag();
	}

}


