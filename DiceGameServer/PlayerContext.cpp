#include "stdafx.h"
#include "PlayerContext.h"

PlayerContext::PlayerContext(std::string userId)
{
	m_userId = userId;
	m_isConnected = true;
	// TODO : set chips to 10000 temp
	m_chips = 10000;
}