#ifndef __zCommonDefine__h__
#define __zCommonDefine__h__


#define PH_LEN               4
#define MAX_MESSAGE_SIZE 1024
#define PACKET_MASK      0x0000ffff
#define MAX_DATABUFFERSIZE (PACKET_MASK+1)

#ifdef WIN32
#define SLEEP(x) Sleep(x)
#else
#define SLEEP(x) usleep(1000*x)
#endif

#endif  //__zCommonDefine__h__

