#include "stdafx.h"
#include "UserSessionManager.h"
#include "zMutex.h"

void UserSessionManager::AddUserById(uint32_t userTempId, UserSession_Ptr session)
{
	zMutex_scope_lock lock_guide(m_userIdMap_lock);
	UserIdMap_Iter iter = m_userIdMap.find(userTempId);
	if (iter != m_userIdMap.end())
	{
		UserSession_Ptr oldSession = iter->second;
		ztLoggerWrite(ZONE, e_Warning, "AddUserById::[%d] Again!!!", userTempId);

		// kick the old user
		oldSession->SetQuit();
		m_userIdMap.erase(iter);
	}
	
	{
		pair<UserIdMap_Iter, bool> bPair =  m_userIdMap.insert(
			make_pair<uint32_t, UserSession_Ptr>(userTempId, session));
		if (bPair.second)
		{
			ztLoggerWrite(ZONE, e_Information, "AddUserById::[%d] OK!!!", userTempId);
		}
		else
		{
			ztLoggerWrite(ZONE, e_Information, "AddUserById::[%d] Add Fail!!!", userTempId);
		}
	}
}

void UserSessionManager::AddUser(const string userId, UserSession_Ptr session)
{
	zMutex_scope_lock lock_guide(m_userMap_lock);
	UserMap_Iter iter = m_userMap.find(userId);
	if (iter != m_userMap.end())
	{
		UserSession_Ptr oldSession = iter->second;
		ztLoggerWrite(ZONE, e_Warning, "AddUser::[%s] Again!!!", userId.c_str());

		// kick the old user
		oldSession->SetQuit();
		m_userMap.erase(iter);
	}
	
	{
		pair<UserMap_Iter, bool> bPair =  m_userMap.insert(
			make_pair<const string, UserSession_Ptr>(userId, session));
		if (bPair.second)
		{
			ztLoggerWrite(ZONE, e_Information, "AddUser::[%s] OK!!!", userId.c_str());
		}
		else
		{
			ztLoggerWrite(ZONE, e_Information, "AddUser::[%s] Add Fail!!!", userId.c_str());
		}
	}
}

void UserSessionManager::RemoveUser(const string userId, uint32_t userTempId)
{
	zMutex_scope_lock lock_guide1(m_userIdMap_lock);
	zMutex_scope_lock lock_guide2(m_userMap_lock);
	
	ztLoggerWrite(ZONE,e_Information, "RemoveSession::[%s] OK!!!", userId.c_str());
	m_userMap.erase(userId);
	m_userIdMap.erase(userTempId);
}

UserSession_Ptr UserSessionManager::getUser(const string userId)
{
	zMutex_scope_lock lock_guide(m_userMap_lock);
	if(userId.length() == 0)
	{
		UserMap_Iter iter = m_userMap.begin();
		return iter->second;
	}
	else
	{
		UserMap_Iter iter = m_userMap.find(userId);
		if (iter != m_userMap.end())
		{
			return iter->second;
		}
	}
	ztLoggerWrite(ZONE, e_Information, "getUser::[%s] Error!!!", userId.c_str());
	return NULL;
}

void UserSessionManager::doCmd()
{
	zMutex_scope_lock lock_guide(m_userIdMap_lock);
	for (UserIdMap_Iter iter = m_userIdMap.begin(); iter != m_userIdMap.end(); ++iter)
	{
		UserSession_Ptr sess =  iter->second;
		if (sess)
		{
			sess->doCmd();
		}

	}
}
