#ifndef __CONFIG_H__
#define __CONFIG_H__
 
#include "zType.h"
#include "zMisc.h"

#include <string>
class ServerConfig  : public SingletonBase<ServerConfig>
{
public:
	int32_t Init (void);
	int32_t Load (void);
	int32_t Reload (void);

 
protected:
	ServerConfig (void);
	~ServerConfig (void);

private:
	ServerConfig (const ServerConfig&);
	ServerConfig& operator= (const ServerConfig&);
	friend class SingletonBase<ServerConfig>;

public:
	std::string  m_strIP;
	int16_t m_sPort;
	int32_t m_iThreadNum;
	int32_t m_iCheckTime;
	std::string m_strDebugLevel;
};


#endif // __CONFIG_H__
