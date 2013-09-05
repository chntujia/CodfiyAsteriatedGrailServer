#ifndef _MESSAGEQUEUE_H_
#define _MESSAGEQUEUE_H_

#include "zMisc.h"
#include <deque>
#include <queue>
//#include <ext/pool_allocator.h>
//#include <ext/mt_allocator.h>
typedef std::pair<unsigned int , unsigned char *> CmdPair;
template <int QueueSize=102400>
class MsgQueue
{
public:
	MsgQueue()
	{
		queueRead=0;
		queueWrite=0;
	}
	~MsgQueue()
	{
		clear();
	}
	//typedef std::pair<volatile bool , CmdPair > CmdQueue;
	typedef std::pair<bool , CmdPair > CmdQueue;
	CmdPair *get()
	{
		CmdPair *ret=NULL;
		if(cmdQueue[queueRead].first)
		{
			ret=&cmdQueue[queueRead].second;
		}
		return ret;
	}
	void erase()
	{
		SAFE_DELETE_VEC(cmdQueue[queueRead].second.second);
		//__mt_alloc.deallocate(cmdQueue[queueRead].second.second, cmdQueue[queueRead].second.first);
		cmdQueue[queueRead].first=false;
		queueRead = (++queueRead)%QueueSize;
	}
	bool put(const void *ptNullCmd, const unsigned int cmdLen)
	{
		unsigned char *buf = new unsigned char[cmdLen];
		//unsigned char *buf = __mt_alloc.allocate(cmdLen);
		if(buf)
		{
			bcopy(ptNullCmd , buf , cmdLen);
			if(!putQueueToArray() && !cmdQueue[queueWrite].first)
			{
				cmdQueue[queueWrite].second.first = cmdLen;
				cmdQueue[queueWrite].second.second = buf;
				cmdQueue[queueWrite].first=true;
				queueWrite = (++queueWrite)%QueueSize;
				return true;
			}
			else
			{
				queueCmd.push(std::make_pair(cmdLen , buf));
			}
			return true;
		}
		return false;

	}
private:
	void clear()
	{
		while(putQueueToArray())
		{
			while(get())
			{
				erase();
			}
		}
		while(get())
		{
			erase();
		}
	}
	bool putQueueToArray()
	{
		bool isLeft=false;
		while(!queueCmd.empty())
		{
			if(!cmdQueue[queueWrite].first)
			{
				cmdQueue[queueWrite].second = queueCmd.front();;
				cmdQueue[queueWrite].first=true;
				queueWrite = (++queueWrite)%QueueSize;
				queueCmd.pop();
			}
			else
			{
				isLeft = true; 
				break;
			}
		}
		return isLeft;
	}
 
	//__gnu_cxx::__mt_alloc<unsigned char> __mt_alloc;
	CmdQueue cmdQueue[QueueSize];
	//#ifdef _POOL_ALLOC_		
	//		std::queue<CmdPair, std::deque<CmdPair, __gnu_cxx::__pool_alloc<CmdPair> > > queueCmd;
	//#else		
	std::queue<CmdPair, std::deque<CmdPair> > queueCmd;
	//#endif		
	unsigned int queueWrite;
	unsigned int queueRead;
};


class MessageQueue
{
	protected:
		virtual ~MessageQueue(){};
	public:
		bool msgParse(const char *ptNullCmd, const unsigned int cmdLen)
		{
			return cmdQueue.put((void*)ptNullCmd , cmdLen);
		}
		virtual bool cmdMsgParse(const char *, const unsigned int)=0;
		bool doCmd()
		{
			CmdPair *cmd = cmdQueue.get();
			while(cmd)
			{
				cmdMsgParse((const char *)cmd->second , cmd->first);
				cmdQueue.erase();
				cmd = cmdQueue.get();
			}
			if(cmd)
			{
				cmdQueue.erase();
			}
			return true;
		}

	private:
		MsgQueue<> cmdQueue;
};

template <int _Size = 20>
class MessageBuffer : public boost::noncopyable
{
        protected:
                virtual ~MessageBuffer(){};
        public: 
                bool put(const char *ptNullCmd, const unsigned int cmdLen) 
                {       
                        return cmdQueue.put((void*)ptNullCmd , cmdLen);
                }       
                virtual bool cmdMsgParse(const char *, const unsigned int)=0; 
                bool doCmd() 
                {       
                        CmdPair *cmd = cmdQueue.get();
                        while(cmd)
                        {       
                                cmdMsgParse((const char *)cmd->second , cmd->first);
                                cmdQueue.erase();
                                cmd = cmdQueue.get();
                        }       
                        if(cmd) 
                        {       
                                cmdQueue.erase();
                        }       
                        return true;
                }       

        private:
                MsgQueue<_Size> cmdQueue;
};
#endif
