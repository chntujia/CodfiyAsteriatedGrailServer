#include "zTCPTask.h"
#include "Config.h"


zTCPTask::zTCPTask(asio::io_service& service)
{
	//m_pSocket = new zSocket(service);
	m_pSocket = new zSocket2(service);
	m_state = TaskState_init;
}

void zTCPTask::Start()
{
	m_pSocket->SetProcessor(this);

	m_pSocket->Start();
	m_state = TaskState_ok;


}
 

 
void zTCPTask::OnQuit()
{
	ztLoggerWrite(ZONE,e_Debug,"zTCPTask::OnQuit ");
	m_state = TaskState_recycle;
}
 

//Message´¦Àíº¯Êý
bool zTCPTask::msgParse(const void *pstrMsg, const uint32_t nCmdLen){return true;}

