#include "Communication.h"

void player_talk(GameGrail* engine, int m_playerId, Talk* talk)
{
	if (talk->txt().at(0) == '!' && talk->txt().at(1) == '`')
	{
		PlayerEntity* player = engine->getPlayerEntity(m_playerId);
		gm_cmd(engine, player, talk->txt());
	}
	else
	{
		if(m_playerId == GUEST){
			return;
		}
		Gossip gossip;
		gossip.set_type(GOSSIP_TALK);
		gossip.set_id(m_playerId);
		gossip.set_txt(talk->txt());
		engine->sendMessage(-1, MSG_GOSSIP, gossip);
	}
}