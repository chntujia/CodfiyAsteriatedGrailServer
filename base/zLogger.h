// Loger.h: interface for the CLoger class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LOGER_H__41F33608_CEB9_47B5_9469_B276C6CDA546__INCLUDED_)
#define AFX_LOGER_H__41F33608_CEB9_47B5_9469_B276C6CDA546__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#pragma warning(disable:4996)
#include <stdio.h>
#include <string>
#include <assert.h>

#include "zMutex.h"

enum ELogLevel
{
	e_Error,
	e_Information,
	e_Warning,
	e_Debug
};

#define ZONE zonestr(__FILE__, __LINE__)
char *zonestr(char *file, int line);
ELogLevel getDebugLevel(const char* msg);

class CZTLoger  
{
public:
	static CZTLoger& Instance();

	void SetFile(const char* filename, ELogLevel level);

	void inline SetLogLevel(ELogLevel level)
	{
		m_Level  = level;
	}

	void Log(const char* zone,
			ELogLevel eLevel,
			 const char* format, 
			 ...);

protected:
	CZTLoger();
	~CZTLoger();
private:
	ELogLevel m_Level;
	void openFileLog();
	FILE*		m_pFile;
	//static CGTLoger* m_pInstance;
	static CZTLoger m_Instance;
	std::string m_strLogName;
	std::string m_strBaseName;
	time_t m_lastWrTime;  //上次写入事件  

	zMutex m_lock;
};

#define _ZT_ENABLE_TRACE
#ifdef _ZT_ENABLE_TRACE
#define ztLoggerInit(x, y) CZTLoger::Instance().SetFile(x, y)
#define ztLoggerWrite 	CZTLoger::Instance().Log
#define ztLoggerSetLevel(x) CZTLoger::Instance().SetLogLevel(x)
#else //_ZT_ENABLE_TRACE
#define ztLoggerInit(x, y) __noop
#define ztLoggerWrite __noop
#define ztLoggerSetLevel(x) __noop
#endif //_ZT_ENABLE_TRACE

#endif // !defined(AFX_LOGER_H__41F33608_CEB9_47B5_9469_B276C6CDA546__INCLUDED_)


