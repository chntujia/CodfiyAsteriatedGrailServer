#ifndef __zType__h__
#define __zType__h__

#include <stdarg.h>
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

#include <string.h>
#define STRNCPY(dst, src, size)			\
	do									\
	{									\
	strncpy(dst, src, (size - 1));	\
	dst[(size - 1)] = '\0';			\
	}									\
	while(0)
 

#ifdef  _MSC_VER
#define bcopy(src,dst,size)	memcpy(dst,src,size)
#define bzero(src,size)		memset(src,0,size)
#endif

#ifdef _MSC_VER
#define strcasecmp _stricmp 
#define snprintf  _snprintf
#endif

#if defined (_MSC_VER)

#if defined (WIN32) || defined (_WIN32)

// typedef char				int8_t;
typedef unsigned char		uint8_t;
typedef short				int16_t;
typedef unsigned short		uint16_t;
typedef int					int32_t;
typedef unsigned int		uint32_t;
typedef long long			int64_t;
typedef unsigned long long	uint64_t;

#else ifdef (_WIN64)

typedef char				int8_t;
typedef unsigned char		uint8_t;
typedef short				int16_t;
typedef unsigned short		uint16_t;
typedef int					int32_t;
typedef unsigned int		uint32_t;
typedef long 				int64_t;
typedef unsigned long		uint64_t;

#endif

#endif	// _MSC_VER

#endif   //__zType__h__
