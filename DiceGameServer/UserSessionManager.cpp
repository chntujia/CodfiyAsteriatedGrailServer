#include "stdafx.h"
#include "UserSessionManager.h"

void UserSessionManager::AddUserById(uint32_t userTempId, UserSession_Ptr session)
{
	zMutex_scope_lock lock_guide(m_userIdMap_lock);	
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

void UserSessionManager::AddUser(const string userId, UserSession_Ptr session)
{
	UserMap_Iter iter = m_userMap.find(userId);
	if (iter != m_userMap.end())
	{
		UserSession_Ptr oldSession = iter->second;
		ztLoggerWrite(ZONE, e_Warning, "AddUser::[%s] Again!!!", userId.c_str());

		// kick the old user
		oldSession->SetQuit();
		m_userMap.erase(iter);
	}
	
	pair<UserMap_Iter, bool> bPair =  m_userMap.insert(
		make_pair<const string, UserSession_Ptr>(userId, session));
	if (bPair.second)
	{
		ztLoggerWrite(ZONE, e_Information, "AddUser::[%s] OK!!!", userId.c_str());
	}
	else
	{
		ztLoggerWrite(ZONE, e_Error, "AddUser::[%s] Add Fail!!!", userId.c_str());
	}

}

void UserSessionManager::RemoveUserById(const uint32_t userTempId)
{	
	ztLoggerWrite(ZONE, e_Information, "RemoveUserById::[%d] OK!!!", userTempId);
	m_userIdMap.erase(userTempId);
}

void UserSessionManager::RemoveUser(const string userId)
{
	zMutex_scope_lock lock_guide(m_userMap_lock);	
	m_userMap.erase(userId);

	ztLoggerWrite(ZONE, e_Information, "RemoveUser::[%s] OK!!!", userId.c_str());
}

void UserSessionManager::trySendMessage(const string userId, uint16_t proto_type, google::protobuf::Message& proto)
{
	zMutex_scope_lock lock_guide(m_userMap_lock);
	UserMap_Iter iter = m_userMap.find(userId);
	if (iter != m_userMap.end()){
		iter->second->sendProto(proto_type, proto); 
	}
	else{
		ztLoggerWrite(ZONE, e_Warning, "UserSessionManager::trySendMessage failed, userId[%s] doesnot exist!!!", userId.c_str());
	}
}

void UserSessionManager::doCmd()
{
	zMutex_scope_lock lock_guide(m_userIdMap_lock);	
	for (UserIdMap_Iter iter = m_userIdMap.begin(); iter != m_userIdMap.end(); ++iter)
	{
		UserSession_Ptr sess =  iter->second;
		sess->doCmd();
	}
}
