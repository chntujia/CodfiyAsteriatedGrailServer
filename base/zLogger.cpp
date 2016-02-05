// Loger.cpp: implementation of the CLoger class.
//
//////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "zType.h"
#include "zLogger.h"

struct stLogMsg 
{
	int level;
	char msg[24];
};

static stLogMsg ELogMsg[] = {{e_Error,"ERROR"},{e_Information,"INFO"},{e_Warning,"WARNING"},{e_Debug,"DEBUG"}};

ELogLevel getDebugLevel(const char* msg)
{
	if(msg == NULL) return e_Debug;
	if (strcasecmp("ERROR",msg) == 0)
	{
		return e_Error;
	}
	else if (strcasecmp("INFO",msg) == 0)
	{
		return e_Information;
	}
	else if (strcasecmp("WARNING",msg) == 0)
	{
		return e_Warning;
	}
	else
	{
		return e_Debug;
	}
}

char *zonestr(char *file, int line)
{
	static char buff[64];

	int i;
	char* p = strrchr(file, '\\');
	i = snprintf(buff,63,"%s:%d", p ? p + 1 : file,line);
	buff[i] = '\0';
	return buff;
}

//#ifdef _ZT_ENABLE_TRACE
CZTLoger CZTLoger::m_Instance;

CZTLoger::CZTLoger()
{
	m_pFile = NULL;
	m_Level = e_Debug;
	m_lastWrTime = time(NULL);
}


CZTLoger::~CZTLoger()
{
	if (m_pFile != stdout && m_pFile != NULL)
	{
		fclose(m_pFile);
	}

	/*
	if (m_pInstance != NULL)
	{
		delete m_pInstance;
		m_pInstance = NULL;
	}*/
}


CZTLoger& CZTLoger::Instance()
{
	/*if (m_pInstance == NULL)
	{
		m_pInstance = new CGTLoger();
	}*/

	return m_Instance;
}


void CZTLoger::SetFile(const char* filename, ELogLevel level)
{
	assert(level <= e_Debug);

	m_Level = level;
	if (filename == NULL)
	{
		m_pFile = stdout;
		return;
	}
	m_strBaseName = filename;

	if (m_pFile != NULL && m_pFile != stdout)
	{
		fclose(m_pFile);
	}

	openFileLog();

	//m_pFile = fopen(filename, "a+t");

	if (m_pFile == NULL)
	{
		m_pFile = stdout;
	}
	else
	{
		m_strLogName = filename;
	}


}

void CZTLoger::openFileLog()
{

	m_lastWrTime = time(NULL);
	if (m_pFile == stdout)
	{
		return;
	}
	std::string strOldLogName = m_strLogName;
	m_strLogName = m_strBaseName;
	//struct tm* ptr = localtime(&m_lastWrTime);
	//char buf[64] = {0};	
	//strftime(buf,64,".%y%m%d-%H",ptr);
	//m_strLogName += buf;
	//if (m_strLogName != strOldLogName)
	//{
	//	if(m_pFile != NULL)
	//		fclose(m_pFile);
	//	m_pFile = fopen(m_strLogName.c_str(),"a+t");
	//}
	//else
	{
		if (m_pFile == NULL)
			m_pFile = fopen(m_strLogName.c_str(),"a+t");
	}
}

void CZTLoger::Log( const char* zone,
				   ELogLevel eLevel,
				   const char* format, 
				   ...)
{
	zMutex_scope_lock scope_lock(m_lock);
	if(format == NULL ||m_pFile == NULL || eLevel > m_Level) return;
	if (m_pFile != stdout)
	{
		openFileLog();	
	}

	va_list args;
	time_t t;
	t = time(NULL);

	va_start(args, format);

	char strCurTime[64];

#ifdef _MSC_VER 
	fprintf(m_pFile,"%s %s %s: ",_strtime(strCurTime),zone,ELogMsg[eLevel].msg);
#else
	//time_t CurTime;
	struct tm ret;
	time_t CurTime = time(NULL);
	localtime_r(&CurTime,&ret);
	strftime(strCurTime,63,"%Y-%m-%d %H:%M:%S ",&ret);
	fprintf(m_pFile,"%s %s %s: ",strCurTime,zone,ELogMsg[eLevel].msg);
#endif
	vfprintf(m_pFile, format, args);
	fprintf(m_pFile,"\n");


	fflush(m_pFile);

	if(eLevel > e_Error) {
		va_end(args);
		return;
	}
	fprintf(stdout,"%s %s %s: ",_strtime(strCurTime),zone,ELogMsg[eLevel].msg);
	vfprintf(stdout,format,args);
	fprintf(stdout,"\n");
	fflush(stdout);
	va_end(args);
}


//#endif //_GT_ENABLE_TRACE

