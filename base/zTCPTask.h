#ifndef __zTCPTask__H__
#define  __zTCPTask__H__
//#include <asio.hpp>
//#include <boost/date_time/posix_time/posix_time.hpp>

//#include <boost/bind.hpp>
//#include <boost/shared_ptr.hpp>
//#include <boost/enable_shared_from_this.hpp>

#include <queue>
#include <string>
#include "zType.h"
#include "zMutex.h"
#include "zLogger.h"
#include "zSocket.h"
#include "zSocket2.h"
using namespace std;
class zTCPTask :  public zProcessor, private boost::noncopyable 
{
public:
	enum TaskState 
	{
		TaskState_init = 0,
		TaskState_ok = 1,
		TaskState_quit = 2,
		TaskState_recycle = 3,
	};


	zTCPTask(asio::io_service& service);

	virtual ~zTCPTask()
	{
		//Close();
		SAFE_DELETE(m_pSocket);
	}
	
	virtual void OnQuit();

	virtual void Start();

	virtual void OnCheck(){};

	//设置Task退出标志
	void SetQuit()
	{
		m_pSocket->SetQuitFlag();
		m_state = TaskState_quit;
	}

	//bool GetQuitFlag()
	//{
	//	return m_pSocket->GetQuitFlag();
	//}

	TaskState GetState(){return m_state;}


	asio::ip::tcp::socket& socket()
	{
		return m_pSocket->socket();
	}

	void Close()
	{
		m_pSocket->Close();
	}


	//Message处理函数
	bool msgParse(const void *pstrMsg, const uint32_t nCmdLen);


	// cmd 加入到sendQueue中
	bool SendCmd(const void* pstrCmd,const int32_t nCmdLen)
	{
		m_pSocket->SendCmd(pstrCmd,nCmdLen);
		return true;
	}

protected:
	 zSocket2  *m_pSocket;
	 TaskState m_state;
	//
	//t_BufferCmdQueue recvQueue;
	//zMutex m_lock;

	//t_BufferCmdQueue sendQueue;				//发送缓冲
	//zMutex m_sendLock;
 
 
};

//typedef boost::shared_ptr<zTCPTask> zTCPTask_Ptr;


#endif  //__zTCPTask__H__

