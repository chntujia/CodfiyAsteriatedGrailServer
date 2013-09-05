#include "tinyxml.h"
#include "Config.h"

ServerConfig::ServerConfig (void)
{
	Init();
}

ServerConfig::~ServerConfig (void)
{
	// do nothing
}

int32_t ServerConfig::Init (void)
{
	m_strIP	= "";
	m_sPort			= 0;
	m_iThreadNum = 4;
	return 0;
}

int32_t ServerConfig::Load (void)
{
	TiXmlDocument kConfig("DiceServerConfig.xml"); 
	if (!kConfig.LoadFile())
	{
		return -1;
	}

	TiXmlHandle kHandle(&kConfig);
	TiXmlElement* pkElement = kHandle.FirstChild("config").FirstChild("server").ToElement();
	if (NULL == pkElement)
	{
		return -1;
	}

	int32_t iValue = 0;
	const char* pszValue = NULL;

	// ip
	pszValue = pkElement->Attribute("ip", &iValue);
	if (NULL == pszValue)
	{
		return -1;

	}
	m_strIP = pszValue;

	// listen port
	pszValue = pkElement->Attribute("port", &iValue);
	if ((NULL == pszValue) || (iValue < 0))
	{
		return -1;
	}
	m_sPort = (int16_t)iValue;

	// thread num
	pszValue = pkElement->Attribute("thread_num", &iValue);
	if ((NULL == pszValue) || (iValue < 0))
	{
		return -1;
	}
	m_iThreadNum = (int32_t)iValue;

	// check_intertime
	pszValue = pkElement->Attribute("check_intertime", &iValue);
	if ((NULL == pszValue) || (iValue < 0))
	{
		return -1;
	}
	m_iCheckTime = (int32_t)iValue;

	// log level
	pszValue = pkElement->Attribute("log_level", &iValue);
	if ((NULL == pszValue))
	{
		return -1;
	}
	m_strDebugLevel = pszValue;

	return 0;
}

int32_t ServerConfig::Reload (void)
{
	TiXmlDocument kConfig("ChatServerConfig.xml"); 
	if (!kConfig.LoadFile())
	{
		return -1;
	}

	TiXmlHandle kHandle(&kConfig);

	TiXmlElement* pkElement = kHandle.FirstChild("config").FirstChild("server").ToElement();
	if (NULL == pkElement)
	{
		return -1;
	}

	int32_t iValue = 0;
	const char* pszValue = NULL;

	// check_intertime
	pszValue = pkElement->Attribute("check_intertime", &iValue);
	if ((NULL == pszValue) || (iValue < 0))
	{
		return -1;
	}
	m_iCheckTime = (int32_t)iValue;

	// log level
	pszValue = pkElement->Attribute("log_level", &iValue);
	if ((NULL == pszValue))
	{
		return -1;
	}
	m_strDebugLevel = pszValue;
 
	return 0;
}



