#ifndef _MISC_H_
#define _MISC_H_
#include <assert.h>
/**
 * \brief 安全删除一个指针
 *
 */
#ifndef SAFE_DELETE
#define SAFE_DELETE(x) { if (x) { delete (x); (x) = NULL; } }
#endif
/**
 * \brief 安全删除一个指针数组
 *
 */
#ifndef SAFE_DELETE_VEC
#define SAFE_DELETE_VEC(x) { if (x) { delete [] (x); (x) = NULL; } }
#endif

template <typename T>
class SingletonBase
{
	public:
		SingletonBase() {}
		virtual ~SingletonBase() {}
		static T& getInstance()
		{
			assert(instance);
			return *instance;
		}
		static void newInstance()
		{
			SAFE_DELETE(instance);
			instance = new T();
		}
		static void delInstance()
		{
			SAFE_DELETE(instance);
		}
	protected:
		static T* instance;
	private:
		SingletonBase(const SingletonBase&);
		SingletonBase & operator= (const SingletonBase &);
};
template <typename T> T* SingletonBase<T>::instance = NULL;

#endif  //_MISC_H_

