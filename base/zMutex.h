#ifndef __zMutex__h__
#define __zMutex__h__

#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>

class zMutex : private boost::noncopyable
{
public:
	zMutex(){}
	~zMutex(){}
	void lock()
	{
		m_mutex.lock();
	}

	void unlock()
	{
		m_mutex.unlock();
	}

	friend class zMutex_scope_lock;

private:
	boost::mutex m_mutex;
};
/*
class zMutex_scope_lock : private boost::noncopyable
{
public:

	explicit zMutex_scope_lock(zMutex &m) : m_lock(m)
	{
		m_lock.lock();
	}

	~zMutex_scope_lock()
	{
		m_lock.unlock();
	}
 
private:
	zMutex &m_lock;
};*/

class zMutex_scope_lock : private boost::noncopyable
{
public:

	explicit zMutex_scope_lock(zMutex &m) : m_scoped_lock(m)
	{
	}

	~zMutex_scope_lock()
	{
	}

private:
	typedef boost::unique_lock<zMutex> scoped_lock;
	scoped_lock m_scoped_lock;
};



#endif  //__zMutex__h__

