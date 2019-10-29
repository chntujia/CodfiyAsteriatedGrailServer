#include "GrailState.h"
#include "GameGrail.h"
#include <Windows.h>
#include <algorithm>
#include <cstdlib> 
#include "DBServices.h"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/format.hpp"

int StateIdle::handle(GameGrail* engine)
{
	Sleep(60 * 1000);
	return GE_SUCCESS;
}

int poll(PollingType type, int optionSize, int threshold, GameGrail* engine)
{
	engine->resetReady(-1);
	PollingRequest msg;
	msg.set_type(type);
	int maxPlayer = engine->getGameMaxPlayers();

	vector<int> counter(optionSize, 0);
	engine->waitForAllConnected(MSG_POLLING_REQ, msg);
	void* reply;
	for (int i = 0; i < maxPlayer; i++) {
		if (GE_SUCCESS == engine->getReply(i, reply)) {
			PollingResponse *respond = (PollingResponse*)reply;
			int opt = respond->option();
			if (opt >= 0 && opt < optionSize) {
				counter[opt]++;
			}
		}
	}
	int maxOpt = 0;
	for (int i = 0; i < optionSize; i++) {
		if (counter[i] > counter[maxOpt]) {
			maxOpt = i;
		}
	}
	maxOpt = counter[maxOpt] > threshold ? maxOpt : -1;
	return maxOpt;
}

int StatePollingDisconnected::handle(GameGrail* engine)
{
	if (engine->getGameNowPlayers() == engine->getGameMaxPlayers()) {
		engine->popGameState();
		return GE_SUCCESS;
	}

	int waitTime = 60 - time(NULL) + beginTime;
	if (waitTime > 0) {
		PollingRequest msg;
		msg.set_type(PollingType::POLLING_FORCE_WAIT);
		PollingRequest * messages[MAXPLAYER];
		int maxPlayer = engine->getGameMaxPlayers();
		for (int i = 0; i < maxPlayer; i++) {
			messages[i] = &msg;
		}
		engine->resetReady(-1);
		engine->waitForAll(MSG_POLLING_REQ, (void**)messages, waitTime);
	}
	
	int opt = poll(PollingType::POLLING_LEGAL_LEAVE, 2, threshold, engine);
	PollingResponse rep;
	rep.set_option(opt);
	engine->sendMessage(-1, MSG_POLLING_REP, rep);

	if (opt == 1) {
		for (int i = 0; i < engine->getGameMaxPlayers(); i++) {
			if (!engine->getPlayerContext(i)->isConnected()) {
				DBInstance.userAccountDAO->gameFlee(engine->getUserId(i));
			}
		}
		engine->discarded = true;	
		engine->popGameState();
		engine->pushGameState(new StateIdle);
	}
	else {
		beginTime = time(NULL);
	}
	return GE_SUCCESS;
}

int StatePollingGameover::handle(GameGrail* engine)
{
	int maxPlayer = engine->getGameMaxPlayers();

	int opt = poll(PollingType::POLLING_MVP, maxPlayer, maxPlayer / 2, engine);
	engine->m_tableLog.mvp = opt;
	DBInstance.statisticDAO->insert(engine->m_tableLog);

	PollingResponse rep;
	rep.set_option(opt);
	engine->sendMessage(-1, MSG_POLLING_REP, rep);

	engine->popGameState();
	engine->pushGameState(new StateIdle);	
	return GE_SUCCESS;
}

int StateWaitForEnter::handle(GameGrail* engine)
{
	if(engine->isAllStartReady())
	{
		engine->popGameState();
		engine->pushGameState(new StateSeatArrange);		
	}
	Sleep(1000);
	return GE_SUCCESS;
}

int StateSeatArrange::handle(GameGrail* engine)
{	
	engine->playing = true;
	int m_maxPlayers = engine->getGameMaxPlayers();
	
	// 直接将随机结果保存到engine中
	GameInfo& game_info = engine->room_info;

	srand ((unsigned int)time(NULL));
	assignTeam(engine);
	vector< int > colors = assignColor(engine->m_seatMode, m_maxPlayers);

	SinglePlayerInfo *player_info;

	for (int i = 0; i < m_maxPlayers; i++) {
		player_info = game_info.add_player_infos();
		int color = colors[i];
		int id;
		if (color) {
			id = red.back();
			red.pop_back();
		}
		else {
			id = blue.back();
			blue.pop_back();
		}
		player_info->set_id(id);
		player_info->set_team(color);
		player_info->set_nickname(engine->getPlayerContext(id)->getName());
	}
	game_info.set_is_started(true);
	engine->sendMessage(-1, MSG_GAME, game_info);
    engine->popGameState();	
	return engine->setStateRoleStrategy();
}

void StateSeatArrange::assignTeam(GameGrail* engine)
{
	int m_maxPlayers = engine->getGameMaxPlayers();
	list< int > red_l = engine->teamA;
	list< int > blue_l = engine->teamB;

	if(rand() % 2)
		red_l.swap(blue_l);

	vector< int > ids;
	for (int i = 0; i < m_maxPlayers; i++)
		ids.push_back(i);
	std::random_shuffle(ids.begin(), ids.end());
	
	for(int i = 0; i < m_maxPlayers; i++){
		int chosen = ids[i];
		if(red_l.end() == std::find(red_l.begin(), red_l.end(), chosen) && blue_l.end() == std::find(blue_l.begin(), blue_l.end(), chosen)){
			if(red_l.size() < m_maxPlayers/2)
				red_l.push_back(chosen);
			else if (blue_l.size() < m_maxPlayers / 2)
				blue_l.push_back(chosen);
		}
	}
	vector< int > red_v( red_l.begin(), red_l.end() );
	vector< int > blue_v( blue_l.begin(), blue_l.end() );
	std::random_shuffle (red_v.begin(), red_v.end());
	std::random_shuffle (blue_v.begin(), blue_v.end());
	
	red = red_v;
	blue = blue_v;
}
 
vector< int > StateSeatArrange::assignColor(int mode, int playerNum)
{
	vector< int > colors;
	switch(mode)
	{
	case SEAT_MODE_RANDOM:
		for(int i = 0; i < playerNum/2; i++){
			colors.push_back(1);
			colors.push_back(0);
		}
		//首位必红
		std::random_shuffle (++colors.begin(), colors.end());	
		break;
	case SEAT_MODE_2COMBO:
	{
		//间隔后，一对前后交换就是二连
		for(int i = 0; i < playerNum; i += 2){
			colors.push_back(1);
			colors.push_back(0);
		}
		//首位必红，在2~(n-1)中随机取一与前一个交换
		int chosen = rand() % (playerNum -2)+ 2;
		int swapped = (chosen-1 + playerNum) % playerNum;
		int temp = colors[chosen];
		colors[chosen] = colors[swapped];
		colors[swapped] = temp;
	}
		break;
	case SEAT_MODE_3COMBO:
	{
		int colors_t[] = {1, 1, 1, 0, 0, 0};
		//首位必红
		int firstRed = rand() % 3;
		int count = 0;		
		for(int i = firstRed; i < 6; i++){
			colors.push_back(colors_t[i]);
		}
		for(int i = 0; i < firstRed; i++){
			colors.push_back(colors_t[i]);
		}
    }
		break;
	case SEAT_MODE_INTERLACE:
		for(int i = 0; i < playerNum/2; i++){
			colors.push_back(1);
			colors.push_back(0);
		}
		break;
	case SEAT_MODE_RBBRRB:
		int colors_t[] = {1, 0, 0, 1, 1, 0};
		for(int i = 0; i < 6; i++){
			colors.push_back(colors_t[i]);
		}
		break;
	}
	return colors;
}

int StateLeaderElection::handle(GameGrail* engine)
{
	if (!isSet) {
		engine->resetReady(-1);
		isSet = true;
	}
	void* reply;
	int maxPlayer = engine->getGameMaxPlayers();
	GameInfo& game_info = engine->room_info;
	bool isTimeOut = !engine->waitForAll(network::MSG_BECOME_LEADER_REQ, (void**)messages);

	for (int i = 0; i < maxPlayer; i++) {
		if (GE_SUCCESS == engine->getReply(i, reply)) {
			BecomeLeaderResponse *respond = (BecomeLeaderResponse*)reply;
			if (respond->yes()) {
				SinglePlayerInfo* player_it = engine->getPlayerInfo(i);				
				if (player_it->team() == RED) {
					red.push_back(i);
				}
				else {
					blue.push_back(i);
				}
			}			
		}
	}

	if (red.empty()) {
		for (int i = 0; i < maxPlayer; i++) {
			SinglePlayerInfo player_it = game_info.player_infos(i);
			if (player_it.team() == RED) {
				red.push_back(player_it.id());
			}
		}
	}
	if (blue.empty()) {
		for (int i = 0; i < maxPlayer; i++) {
			SinglePlayerInfo player_it = game_info.player_infos(i);
			if (player_it.team() == BLUE) {
				blue.push_back(player_it.id());
			}
		}
	}

	int rl = red[rand() % red.size()];
	int bl = blue[rand() % blue.size()];
	engine->getPlayerInfo(rl)->set_leader(1);
	engine->getPlayerInfo(bl)->set_leader(1);

	GameInfo update;
	SinglePlayerInfo *player;
	player = update.add_player_infos();
	player->set_id(rl);
	player->set_leader(1);
	player = update.add_player_infos();
	player->set_id(bl);
	player->set_leader(1);

	engine->sendMessage(-1, MSG_GAME, update);
	engine->popGameState();
	return isTimeOut ? GE_TIMEOUT : GE_SUCCESS;
}

int StateRoleStrategyRandom::handle(GameGrail* engine)
{
	Deck* roles = engine->initRoles();
	int ret;
	// 直接将随机结果保存到engine中
	GameInfo& game_info = engine->room_info;
	int roleID;

	for(int i = 0; i < engine->getGameMaxPlayers(); i++){
		// i为玩家编号，不是座号		
		if(GE_SUCCESS == (ret=roles->pop(1, &roleID))){
			Coder::roleNotice(i, roleID, game_info);
		}
		else{
			return ret;
		}
	}
	game_info.set_is_started(true);
	engine->sendMessage(-1, MSG_GAME, game_info);
	SAFE_DELETE(roles);
	engine->initPlayerEntities();
	engine->popGameState();
	engine->pushGameState(new StateGameStart);
	return GE_SUCCESS;
}

int StateRoleStrategy31::handle(GameGrail* engine)
{
	int ret;	
	if(!isSet){
		int options[3];
		Deck* roles = engine->initRoles();
		for(int i = 0; i < engine->getGameMaxPlayers(); i++){
			if(GE_SUCCESS == (ret = roles->pop(3, options))){
				messages[i] = new RoleRequest;
				Coder::askForRole(i, 3, options, *messages[i]);
			}
			else{
				return ret;
			}
		}
		engine->resetReady(-1);
		SAFE_DELETE(roles);
		isSet = true;
	}
	void* reply;
	int chosen;
	GameInfo& game_info = engine->room_info;
	bool isTimeOut = !engine->waitForAll(network::MSG_ROLE_REQ, (void**)messages);
			
	for(int i = 0; i < engine->getGameMaxPlayers(); i++){	
		if(GE_SUCCESS == (ret = engine->getReply(i, reply))){
			PickBan *respond = (PickBan*)reply;
			chosen = respond->role_ids(0);
			Coder::roleNotice(i, chosen, game_info);
		}
		else{
			chosen = messages[i]->role_ids(0);
			Coder::roleNotice(i, chosen, game_info);
		}
	}
	game_info.set_is_started(true);
	engine->sendMessage(-1, MSG_GAME, game_info);	
	engine->initPlayerEntities();
	engine->popGameState();
	engine->pushGameState(new StateGameStart);
	return isTimeOut ? GE_TIMEOUT : GE_SUCCESS;
}

int StateRoleStrategyAny::handle(GameGrail* engine)
{
	int ret;
	if(!isSet){
		for(int i = 0; i < engine->getGameMaxPlayers(); i++){
			messages[i] = new RoleRequest;
			Coder::askForRole(i, sizeof(SUMMON)/sizeof(int), SUMMON, *messages[i]);
		}
		engine->resetReady(-1);
		isSet = true;
	}
	void* reply;
	int chosen;
	GameInfo& game_info = engine->room_info;
	bool isTimeOut = !engine->waitForAll(network::MSG_ROLE_REQ, (void**)messages);
			
	for(int i = 0; i < engine->getGameMaxPlayers(); i++){	
		if(GE_SUCCESS == (ret = engine->getReply(i, reply))){
			PickBan *respond = (PickBan*)reply;
			chosen = respond->role_ids(0);
			Coder::roleNotice(i, chosen, game_info);
		}
		else{
			Coder::roleNotice(i, i + 1, game_info);
		}
	}
	game_info.set_is_started(true);
	engine->sendMessage(-1, MSG_GAME, game_info);

	engine->initPlayerEntities();
	engine->popGameState();
	engine->pushGameState(new StateGameStart);
	return isTimeOut ? GE_TIMEOUT : GE_SUCCESS;
}

int StateRoleStrategyBP::handle(GameGrail* engine)
{
	Deck* roles;
	
	int ret = 0;
	GameInfo& game_info = engine->room_info;
	if(step == 0){
		playerNum = engine->getGameMaxPlayers();
		alternativeNum = BP_ALTERNATIVE_NUM[playerNum/2-2];
		if(alternativeNum <= 0)
			return GE_FATAL_ERROR;
		alternativeRoles = new int[alternativeNum];
		options = new int[alternativeNum];
		memset(alternativeRoles, 0, sizeof(int)* alternativeNum);
		memset(options, 0, sizeof(int)* alternativeNum);
		roles = engine->initRoles();
		GameInfo& room_info = engine->room_info;
		SinglePlayerInfo* player_it;
		for(int i = 0; i < playerNum; i++){
			player_it = (SinglePlayerInfo*)&(room_info.player_infos().Get(i));
			int color = player_it->team();
			if(color == RED)
				red.push_back(player_it->id());
			else
				blue.push_back(player_it->id());
		}
		RoleRequest message;
		if(GE_SUCCESS == (ret = roles->pop(alternativeNum, alternativeRoles))){
			Coder::setAlternativeRoles(-1, alternativeNum, alternativeRoles, options, message);
			message.set_opration(BP_NULL);
		}
		else{
			return ret;
		}
		engine->sendMessage(-1, network::MSG_ROLE_REQ, message);
		step = 1;
	}
	while(step > 0 && step <= (playerNum*2)) {
		int color = step % 2;
		int index = (step-1)/4;
		int isPick = ((step-1)/2)%2;
		int id = 0;
		if(color == 1)
			id = red[index];
		else
			id = blue[index];
		if(isPick == 0)
		{
			RoleRequest message;
			Coder::setAlternativeRoles(id, alternativeNum, alternativeRoles, options, message);
			message.set_opration(BP_BAN);
			bool recieved = engine->waitForOne(id, network::MSG_ROLE_REQ, message);
			if(recieved)
			{
				void* reply;
				int ret;
				if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
				{
					PickBan* respond = (PickBan*) reply;
					int chosen = respond->role_ids(0);
					for(int i = 0; i < alternativeNum; ++ i)
						if(alternativeRoles[i] == chosen)
						{
							options[i] = step;
							break;
						}
				}
			}
			else
				return GE_TIMEOUT;
		}
		else
		{
			RoleRequest message;
			Coder::setAlternativeRoles(id, alternativeNum, alternativeRoles, options, message);
			message.set_opration(BP_PICK);
			bool recieved = engine->waitForOne(id, network::MSG_ROLE_REQ, message);
			if(recieved)
			{
				void* reply;
				int ret;
				if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
				{
					PickBan* respond = (PickBan*) reply;
					int chosen = respond->role_ids(0);
					Coder::roleNotice(id, chosen, game_info);
					for(int i = 0; i < alternativeNum; ++ i)
						if(alternativeRoles[i] == chosen)
						{
							options[i] = step;
							break;
						}
					engine->sendMessage(-1, MSG_GAME, game_info);
				}
			}
			else
				return GE_TIMEOUT;
		}
		RoleRequest message;
		Coder::setAlternativeRoles(id, alternativeNum, alternativeRoles, options, message);
		message.set_opration(BP_NULL);
		engine->sendMessage(-1, network::MSG_ROLE_REQ, message);		
		step++;
	}

	engine->initPlayerEntities();
	engine->popGameState();
	engine->pushGameState(new StateGameStart);
	return ret ? GE_TIMEOUT : GE_SUCCESS;
}

int StateRoleStrategyCM::handle(GameGrail* engine)
{
	Deck* roles;

	int ret = 0;

	GameInfo& game_info = engine->room_info;
	if (step == 0) {
		decided = false;
		ibb = 1;
		ibr = 1;
		playerNum = engine->getGameMaxPlayers();
		alternativeNum = BP_ALTERNATIVE_NUM[playerNum / 2 - 2];
		if (alternativeNum <= 0)
			return GE_FATAL_ERROR;
		alternativeRoles = new int[alternativeNum];
		options = new int[alternativeNum];
		memset(alternativeRoles, 0, sizeof(int)* alternativeNum);
		memset(options, 0, sizeof(int)* alternativeNum);
		roles = engine->initRoles();
		GameInfo& room_info = engine->room_info;
		for (int i = 0; i < playerNum; i++) {
			SinglePlayerInfo player_it = room_info.player_infos().Get(i);
			int leader = player_it.leader();
			int color = player_it.team();
			if (leader == 1)
			{
				if (color == RED)
					idr = player_it.id();
				else  
					idb = player_it.id();
			}
			if (color == RED)
			{
				red.push_back(player_it.id());
			}
			else
			{
				blue.push_back(player_it.id());
			}
		}
		RoleRequest message;
		if (GE_SUCCESS == (ret = roles->pop(alternativeNum, alternativeRoles))) {
			Coder::setCMRoles(-1, alternativeNum, alternativeRoles, options, message);
			message.set_opration(CM_NULL);
		}
		else {
			return ret;
		}
		engine->sendMessage(-1, network::MSG_ROLE_REQ, message);
		step = 1;
	}
	while (step > 0 && step <= 9) {
		int id1 = 0; int id2 = 0; int id = 0;
		if (step == 1)
		{
			RoleRequest message;
			Coder::setCMRoles(idr, alternativeNum, alternativeRoles, options, message);
			message.set_opration(CM_RED_BAN);
			bool recieved = engine->waitForOne(idr, network::MSG_ROLE_REQ, message);
			if (recieved)
			{
				void* reply;
				int ret;
				if (GE_SUCCESS == (ret = engine->getReply(idr, reply)))
				{
					PickBan* respond = (PickBan*)reply;
					int chosen = respond->role_ids(0);
					for (int i = 0; i < alternativeNum; ++i)
						if (alternativeRoles[i] == chosen)
						{
							options[i] = 12;
							break;
						}
				}
			}
			else
				return GE_TIMEOUT;
		}
		else if (step == 2 || step == 3)
		{
			RoleRequest message;
			Coder::setCMRoles(idb, alternativeNum, alternativeRoles, options, message);
			message.set_opration(CM_BLUE_BAN);
			bool recieved = engine->waitForOne(idb, network::MSG_ROLE_REQ, message);
			if (recieved)
			{
				void* reply;
				int ret;
				if (GE_SUCCESS == (ret = engine->getReply(idb, reply)))
				{
					PickBan* respond = (PickBan*)reply;
					int chosen = respond->role_ids(0);
					for (int i = 0; i < alternativeNum; ++i)
						if (alternativeRoles[i] == chosen)
						{
							options[i] = 15;
							break;
						}
				}
			}
			else
				return GE_TIMEOUT;
		}
		else
		{
			int color = step % 2;
			int index = (step - 4) / 2;
			bool ibable;
			if (color == 0) { id1 = idr; id2 = idb; ibable = ibb; id = red[index]; }
			else { id1 = idb; id2 = idr; ibable = ibr; id = blue[index]; }
			if (ibable && !decided) {
				if (id2 == idr && ibr == 1)
				{
					RoleRequest message;
					Coder::setCMRoles(idr, alternativeNum, alternativeRoles, options, message);
					message.set_opration(CM_RED_IB);
					bool recieved = engine->waitForOne(idr, network::MSG_ROLE_REQ, message);
					if (recieved)
					{
						decided = true;
						void* reply;
						int ret;
						if (GE_SUCCESS == (ret = engine->getReply(idr, reply)))
						{
							PickBan* respond = (PickBan*)reply;
							int chosen = respond->role_ids(0);
							if (chosen != 100)
							{
								for (int i = 0; i < alternativeNum; ++i)
								{
									if (alternativeRoles[i] == chosen)
									{
										options[i] = 13;
										break;
									}
								}
								ibr = 0;
							}
						}
					}
					else
						return GE_TIMEOUT;
				}
				else if (id2 == idb && ibb == 1)
				{
					RoleRequest message;
					Coder::setCMRoles(idb, alternativeNum, alternativeRoles, options, message);
					message.set_opration(CM_BLUE_IB);
					bool recieved = engine->waitForOne(idb, network::MSG_ROLE_REQ, message);
					if (recieved)
					{
						decided = true;
						void* reply;
						int ret;
						if (GE_SUCCESS == (ret = engine->getReply(idb, reply)))
						{

							PickBan* respond = (PickBan*)reply;
							int chosen = respond->role_ids(0);
							if (chosen != 100)
							{
								for (int i = 0; i < alternativeNum; ++i)
								{
									if (alternativeRoles[i] == chosen)
									{
										options[i] = 16;
										break;
									}
								}
								ibb = 0;
							}
						}
					}
					else
						return GE_TIMEOUT;
				}
			}
			else {
				RoleRequest message;
				Coder::setCMRoles(id1, alternativeNum, alternativeRoles, options, message);
				if (id1 == idr) { message.set_opration(CM_RED_PICK); }
				else message.set_opration(CM_BLUE_PICK);
				bool recieved = engine->waitForOne(id1, network::MSG_ROLE_REQ, message);
				if (recieved)
				{
					decided = false;
					void* reply;
					int ret;
					if (GE_SUCCESS == (ret = engine->getReply(id1, reply)))
					{
						PickBan* respond = (PickBan*)reply;
						int chosen = respond->role_ids(0);
						Coder::roleNotice(id, chosen, game_info);
						for (int i = 0; i < alternativeNum; ++i)
							if (alternativeRoles[i] == chosen)
							{
								options[i] = (id1 == idr) ? 14 : 17;
								break;
							}
						engine->sendMessage(-1, MSG_GAME, game_info);
					}
				}
				else
					return GE_TIMEOUT;
			}
		}
		RoleRequest message;
		int idn;
		if (step == 1)idn = idr;
		else if (step == 2 || step == 3)idn = idb;
		else
			idn = decided ? id2 : id1;
		Coder::setCMRoles(idn, alternativeNum, alternativeRoles, options, message);
		message.set_opration(CM_NULL);
		engine->sendMessage(-1, network::MSG_ROLE_REQ, message);
		for (int i = 0; i < alternativeNum; ++i) {
			if (options[i] > 10)options[i] -= 10;
		}
		if (!decided)
			step++;

	}
	
	engine->initPlayerEntities();
	engine->popGameState();
	engine->pushGameState(new StateGameStart);
	return ret ? GE_TIMEOUT : GE_SUCCESS;

}

int StateGameStart::handle(GameGrail* engine)
{	
	if(!isSet){		
		isSet=true;		
		Sleep(1000);
	}
	int ret;
	vector< int > cards;
	PlayerEntity* tempEntity;
	HARM harm;
	harm.type = HARM_NONE;
	harm.point = 4;
	harm.srcID = -1;
	harm.cause = CAUSE_DEFAULT;
	//LIFO
	for(int i = iterator; i < engine->getGameMaxPlayers(); i++){
		ret = engine->setStateMoveCardsToHand(-1, DECK_PILE, i, DECK_HAND, 4, cards, harm, false);
		iterator++;
		return ret;
	}	
	engine->popGameState();		

	engine->m_tableLog.tableMode = engine->m_roleStrategy;
	engine->m_tableLog.playerNums = engine->getGameMaxPlayers();
	boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
	const boost::wformat f = boost::wformat(L"%s-%02d-%02d %02d:%02d:%02d")
		% now.date().year_month_day().year
		% now.date().year_month_day().month.as_number()
		% now.date().year_month_day().day.as_number()

		% now.time_of_day().hours()
		% now.time_of_day().minutes()
		% now.time_of_day().seconds();

	const std::wstring result = f.str();
	engine->m_tableLog.createTime = string(result.begin(), result.end()); 

	for (int i = 0; i < engine->m_tableLog.playerNums; i++) //根据人数写入细节数据 
	{
		tempEntity = engine->getPlayerEntity(i);
		engine->m_tableLog.tableDetail[i].playerID = engine->getUserId(i);
		engine->m_tableLog.tableDetail[i].playerSerial = tempEntity->getID();
		engine->m_tableLog.tableDetail[i].role = tempEntity->getRoleID();
		engine->m_tableLog.tableDetail[i].team = tempEntity->getColor();

		DBInstance.userAccountDAO->gameStart(engine->m_tableLog.tableDetail[i].playerID);
	}
		
	return engine->setStateCurrentPlayer(engine->m_firstPlayerID);
}

int StateBeforeTurnBegin::handle(GameGrail* engine)
{
	int ret = GE_FATAL_ERROR;
	int m_currentPlayerID = engine->getCurrentPlayerID();
	PlayerEntity *handler = NULL;
	while (iterator < engine->getGameMaxPlayers()){	   
		handler = engine->getNextPlayerEntity(handler, iterator, step);
		ret = handler->p_before_turn_begin(step, m_currentPlayerID);
		moveIterator(ret);
		if(GE_SUCCESS != ret){
			return ret;
		}		
	}
	if(GE_SUCCESS == (ret = engine->popGameState_if(STATE_BEFORE_TURN_BEGIN))){
		engine->pushGameState(new StateTurnBegin);
	}
	return ret;
}

int StateTurnBegin::handle(GameGrail* engine)
{
	int ret = GE_FATAL_ERROR;
	int m_currentPlayerID= engine->getCurrentPlayerID();
	PlayerEntity *handler = NULL;
	while (iterator < engine->getGameMaxPlayers()) {
		handler = engine->getNextPlayerEntity(handler, iterator, step);
		ret = handler->p_turn_begin(step, m_currentPlayerID);
		moveIterator(ret);
		if(GE_SUCCESS != ret){
			return ret;
		}		
	}
	if(GE_SUCCESS == (ret = engine->popGameState_if(STATE_TURN_BEGIN))){
		engine->pushGameState(new StateTurnBeginShiRen);
	}
	return ret;
}

int StateTurnBeginShiRen::handle(GameGrail* engine)
{
	int ret = GE_FATAL_ERROR;
	int m_currentPlayerID= engine->getCurrentPlayerID();
	PlayerEntity *handler = NULL;
	while (iterator < engine->getGameMaxPlayers()) {
		handler = engine->getNextPlayerEntity(handler, iterator, step);
		ret = handler->p_turn_begin_shiren(step, m_currentPlayerID);
		moveIterator(ret);
		if(GE_SUCCESS != ret){
			return ret;
		}		
	}
	if(GE_SUCCESS == (ret = engine->popGameState_if(STATE_TURN_BEGIN_SHIREN))){
		return engine->setStateCheckBasicEffect();
	}
	return ret;
}

int StateWeaken::handle(GameGrail* engine)
{
	int m_currentPlayerID = engine->getCurrentPlayerID();

	CommandRequest weaken_proto;
	Coder::askForWeak(m_currentPlayerID, howMany, weaken_proto);
	if(engine->waitForOne(m_currentPlayerID, network::MSG_CMD_REQ, weaken_proto))
	{
		void* reply;
		int ret;
		if(GE_SUCCESS == (ret=engine->getReply(m_currentPlayerID, reply)))
		{
			Respond *weak_respond = (Respond*)reply;

			if(weak_respond->args().Get(0) == 1){
				int srcID_t = srcID;
				int howMany_t = howMany;
				engine->popGameState();
				engine->pushGameState(new StateBeforeAction);
				engine->pushGameState(new StateBetweenWeakAndAction);
				HARM harm;
				harm.type = HARM_NONE;
				harm.point = howMany_t;
				harm.srcID = srcID_t;
				harm.cause = CAUSE_WEAKEN;
				vector< int > cards;
				engine->setStateMoveCardsToHand(-1, DECK_PILE, m_currentPlayerID, DECK_HAND, howMany_t, cards, harm, false);
				return GE_SUCCESS;
			}
			else{
				engine->popGameState();
				engine->pushGameState(new StateTurnEnd);
				engine->pushGameState(new StateBetweenWeakAndAction);
				return GE_SUCCESS;
			}
		}
		return ret;
	}
	else
	{
		//Timeout, skip to nextState
		Respond weak_respond;
		weak_respond.set_src_id(m_currentPlayerID);
		weak_respond.add_args(0);

		engine->sendMessage(-1, MSG_RESPOND, weak_respond);
		engine->popGameState();
		engine->pushGameState(new StateTurnEnd);
		engine->pushGameState(new StateBetweenWeakAndAction);
		return GE_TIMEOUT;
	}

}

int StateBetweenWeakAndAction::handle(GameGrail* engine)
{
	int ret = GE_FATAL_ERROR;
	int m_currentPlayerID = engine->getCurrentPlayerID();
	PlayerEntity *handler = NULL;
	while (iterator < engine->getGameMaxPlayers()) {
		handler = engine->getNextPlayerEntity(handler, iterator, step);
		ret = handler->p_between_weak_and_action(step, m_currentPlayerID);
		moveIterator(ret);
		if(GE_SUCCESS != ret){
			return ret;
		}		
	}
	ret = engine->popGameState_if(STATE_BETWEEN_WEAK_AND_ACTION);
	return ret;
}

int StateBeforeAction::handle(GameGrail* engine)
{
	int ret = GE_FATAL_ERROR;
	int m_currentPlayerID = engine->getCurrentPlayerID();
	PlayerEntity *handler = NULL;
	while (iterator < engine->getGameMaxPlayers()) {
		handler = engine->getNextPlayerEntity(handler, iterator, step);
		ret = handler->p_before_action(step, m_currentPlayerID);
		moveIterator(ret);
		if(GE_SUCCESS != ret){
			return ret;
		}		
	}
	if(GE_SUCCESS == (ret = engine->popGameState_if(STATE_BEFORE_ACTION))){
		engine->pushGameState(new StateBoot);
	}

	return ret;
}

int StateBoot::handle(GameGrail* engine)
{
	int ret = GE_FATAL_ERROR;
	int m_currentPlayerID = engine->getCurrentPlayerID();
	PlayerEntity *handler = NULL;
	while (iterator < engine->getGameMaxPlayers()) {
		handler = engine->getNextPlayerEntity(handler, iterator, step);
		ret = handler->p_boot(step, m_currentPlayerID);
		moveIterator(ret);
		if(GE_SUCCESS != ret){
			return ret;
		}		
	}
	if(GE_SUCCESS == (ret = engine->popGameState_if(STATE_BOOT))){
		engine->pushGameState(new StateActionPhase(ACTION_ANY, false));
	}
	return ret;
}

int StateActionPhase::handle(GameGrail* engine)
{
	int m_currentPlayerID = engine->getCurrentPlayerID();

	PlayerEntity *src = engine->getPlayerEntity(m_currentPlayerID);
	if( GE_SUCCESS == src->checkExclusiveEffect(EX_TIAO_XIN)){//[Yongzhe]挑衅
		allowAction = ACTION_ATTACK;
		canGiveUp = true;
	}

	CommandRequest cmd_req;
	Coder::askForAction(m_currentPlayerID, allowAction, canGiveUp, cmd_req);
	if(engine->waitForOne(m_currentPlayerID, MSG_CMD_REQ, cmd_req))
	{
		void* temp;
		int ret;
		if(GE_SUCCESS == (ret = engine->getReply(m_currentPlayerID, temp))){
			Action *action = (Action*) temp;
			

			if(GE_SUCCESS != (ret = src->v_allow_action(action, allowAction, canGiveUp))){
				return ret;
			}

			switch(action->action_type())
			{
			case ACTION_ATTACK:
				return basicAttack(action, engine);
			case ACTION_MAGIC:
				return basicMagic(action, engine);
			case ACTION_SPECIAL:
				return basicSpecial(action, engine);
			case ACTION_MAGIC_SKILL:
				return magicSkill(action, engine);
			case ACTION_ATTACK_SKILL:
				return attackSkill(action, engine);
			case ACTION_SPECIAL_SKILL:
				return specialSkill(action, engine);
			case ACTION_UNACTIONAL:
				return unactional(action, engine);
			case ACTION_NONE:
				engine->popGameState_if(STATE_ACTION_PHASE);
			    return engine->setStateCheckTurnEnd();
			default:
				return GE_INVALID_ACTION;
			}
		}
		return ret;
	}
	else{
		if(canGiveUp){
			engine->popGameState_if(STATE_ACTION_PHASE);
			engine->setStateCheckTurnEnd();
		}
		return GE_TIMEOUT;
	}
}

int StateActionPhase::unactional(Action *action, GameGrail* engine)
{
	int currentPlayerId = action->src_id();
	PlayerEntity* player = engine->getPlayerEntity(currentPlayerId);
	list< int > cardList = player->getHandCards();
	vector< int > cards(cardList.begin(), cardList.end());
	int howMany = cards.size();
	if(howMany > 0){
		HARM unactional;
		unactional.cause = CAUSE_UNACTIONAL;
		unactional.point = howMany;
		unactional.srcID = currentPlayerId;
		unactional.type = HARM_NONE;

		engine->sendMessage(-1, MSG_ACTION, *action);
		CardMsg show_card;
		Coder::showCardNotice(currentPlayerId, howMany, cards, show_card);
		engine->sendMessage(-1, MSG_CARD, show_card);

		engine->setStateMoveCardsToHand(-1, DECK_PILE, currentPlayerId, DECK_HAND, howMany, vector< int >(), unactional, false);
		engine->setStateMoveCardsNotToHand(currentPlayerId, DECK_HAND, -1, DECK_DISCARD, howMany, cards, currentPlayerId, CAUSE_UNACTIONAL, true);
		return GE_SUCCESS;
	}
	else{
		int color = player->getColor();
		engine->sendMessage(-1, MSG_ACTION, *action);
		engine->popGameState_if(STATE_ACTION_PHASE);
		engine->getTeamArea()->setMorale(color, 0);
		// 更新士气
		GameInfo update_info;
		if (color == RED)
			update_info.set_red_morale(0);
		else
			update_info.set_blue_morale(0);
		engine->sendMessage(-1, MSG_GAME, update_info);
		engine->pushGameState(new StateGameOver(1 - color));
		return GE_SUCCESS;
	}
}

int StateActionPhase::basicAttack(Action *action, GameGrail* engine)
{
	int ret;
	int card_id = action->card_ids(0);
	int m_currentPlayerID = engine->getCurrentPlayerID();
	PlayerEntity *src = engine->getPlayerEntity(m_currentPlayerID);
	if(GE_SUCCESS == (ret = src->v_attack(card_id, action->dst_ids().Get(0)))){
		engine->popGameState();
		return engine->setStateAttackAction(card_id, action->dst_ids().Get(0), action->src_id());
	}
	return ret;
}

int StateActionPhase::basicMagic(Action *action, GameGrail* engine)
{
	int ret;
	int card_id = action->card_ids(0);
	int m_currentPlayerID = engine->getCurrentPlayerID();
	PlayerEntity *src = engine->getPlayerEntity(m_currentPlayerID);
	PlayerEntity *dst = engine->getPlayerEntity(action->dst_ids(0));
	switch(getCardByID(card_id)->getName())
	{
	case NAME_POISON:
		if (GE_SUCCESS == (ret = src->checkOneHandCard(card_id))){
			engine->popGameState();
			engine->pushGameState(new StateAfterMagic(m_currentPlayerID));
			engine->setStateUseCard(card_id, action->dst_ids(0), m_currentPlayerID, true);
			engine->pushGameState(new StateBeforeMagic(m_currentPlayerID));
			return GE_SUCCESS;
		}
		break;
	case NAME_SHIELD:
		if (GE_SUCCESS == (ret = src->v_shield(card_id, dst))){
			engine->popGameState();
			engine->pushGameState(new StateAfterMagic(m_currentPlayerID));
			engine->setStateUseCard(card_id, action->dst_ids(0), m_currentPlayerID, true);
			engine->pushGameState(new StateBeforeMagic(m_currentPlayerID));
			return GE_SUCCESS;
		}
		break;
	case NAME_MISSILE:
		if (GE_SUCCESS == (ret = src->v_missile(card_id, action->dst_ids(0)))){
			engine->popGameState();
			engine->pushGameState(new StateAfterMagic(m_currentPlayerID));
			engine->pushGameState(StateMissiled::create(engine, card_id, action->dst_ids(0), m_currentPlayerID));
			engine->setStateUseCard(card_id, action->dst_ids(0), m_currentPlayerID);
			engine->pushGameState(new StateBeforeMagic(m_currentPlayerID));
			return GE_SUCCESS;
		}
		break;
	case NAME_WEAKEN:
		if (GE_SUCCESS == (ret = src->v_weaken(card_id, dst))){
			engine->popGameState();
			engine->pushGameState(new StateAfterMagic(m_currentPlayerID));
			engine->setStateUseCard(card_id, action->dst_ids(0), m_currentPlayerID, true);
			engine->pushGameState(new StateBeforeMagic(m_currentPlayerID));
			return GE_SUCCESS;
		}
	default:
		return GE_INVALID_ACTION;
	}
	return ret;
}

int StateActionPhase::basicSpecial(Action *action, GameGrail* engine)
{
	int ret;
	int m_currentPlayerID = engine->getCurrentPlayerID();
	PlayerEntity *src = engine->getPlayerEntity(m_currentPlayerID);
	PlayerEntity *dst;
	switch(action->action_id())
	{
	case SPECIAL_BUY:
		if (GE_SUCCESS == (ret = src->v_buy(action))){
			engine->sendMessage(-1, MSG_ACTION, *action);
			GameInfo update_info;
			TeamArea *team = engine->getTeamArea();
			int color = src->getColor();
			team->setGem(color, team->getGem(color) + action->args(0));
			team->setCrystal(color, team->getCrystal(color) + action->args(1));
			if (color == RED){
				update_info.set_red_gem(team->getGem(color));
				update_info.set_red_crystal(team->getCrystal(color));
			}
			else{
				update_info.set_blue_gem(team->getGem(color));
				update_info.set_blue_crystal(team->getCrystal(color));
			}
			engine->sendMessage(-1, MSG_GAME, update_info);
			engine->popGameState();
			engine->pushGameState(new StateAfterSpecial(m_currentPlayerID));
			vector<int> cards;
			HARM buy;
			buy.cause = CAUSE_BUY;
			buy.point = 3;
			buy.srcID = m_currentPlayerID;
			buy.type = HARM_NONE;
			engine->setStateMoveCardsToHand(-1, DECK_PILE, m_currentPlayerID, DECK_HAND, 3, cards, buy, false);
			engine->pushGameState(new StateBeforeSpecial(m_currentPlayerID));
			return GE_SUCCESS;
		}
		break;
	case SPECIAL_SYNTHESIZE:
		if (GE_SUCCESS == (ret = src->v_synthesize(action, engine->getTeamArea()))){
			engine->sendMessage(-1, MSG_ACTION, *action);
			GameInfo update_info;
			TeamArea *team = engine->getTeamArea();
			int color = src->getColor();
			int gem = action->args(0);
			int crystal = action->args(1);
			team->setGem(color, team->getGem(color)-gem);
			team->setCrystal(color, team->getCrystal(color)-crystal);
			team->setCup(color, team->getCup(color)+1);
			if (color == RED){
				update_info.set_red_gem(team->getGem(color));
				update_info.set_red_crystal(team->getCrystal(color));
				update_info.set_red_grail(team->getCup(color));
			}
			else{
				update_info.set_blue_gem(team->getGem(color));
				update_info.set_blue_crystal(team->getCrystal(color));
				update_info.set_blue_grail(team->getCup(color));
			}
			engine->sendMessage(-1, MSG_GAME, update_info);
			engine->popGameState();
			engine->pushGameState(new StateAfterSpecial(m_currentPlayerID));
			CONTEXT_LOSE_MORALE* morale = new CONTEXT_LOSE_MORALE;
			dst = src;
			while(dst->getColor() == src->getColor()){
				dst = dst->getPost();
			}
			if(team->getCup(color) == 5){
				engine->pushGameState(new StateGameOver(color));  
			}
			HARM grail;
			grail.cause = CAUSE_SYNTHESIZE;
			grail.point = 1;
			grail.srcID = m_currentPlayerID;
			grail.type = HARM_NONE;
			morale->dstID = dst->getID();
			morale->harm = grail;
			morale->howMany = 1;
			engine->pushGameState(new StateBeforeLoseMorale(morale));
			vector<int> cards;
			HARM synthesize;
			synthesize.cause = CAUSE_SYNTHESIZE;
			synthesize.point = 3;
			synthesize.srcID = m_currentPlayerID;
			synthesize.type = HARM_NONE;
			engine->setStateMoveCardsToHand(-1, DECK_PILE, m_currentPlayerID, DECK_HAND, 3, cards, synthesize, false);
			engine->pushGameState(new StateBeforeSpecial(m_currentPlayerID));
			return GE_SUCCESS;
		}
		break;
	case SPECIAL_EXTRACT:
		if (GE_SUCCESS == (ret = src->v_extract(action, engine->getTeamArea()))){
			engine->sendMessage(-1, MSG_ACTION, *action);
			GameInfo update_info;
			TeamArea *team = engine->getTeamArea();
			int color = src->getColor();
			int gem = action->args(0);
			int crystal = action->args(1);
			team->setGem(color, team->getGem(color)-gem);
			team->setCrystal(color, team->getCrystal(color)-crystal);
			src->setGem(src->getGem()+gem);
			src->setCrystal(src->getCrystal()+crystal);
			if (color == RED){
				update_info.set_red_gem(team->getGem(color));
				update_info.set_red_crystal(team->getCrystal(color));
			}
			else{
				update_info.set_blue_gem(team->getGem(color));
				update_info.set_blue_crystal(team->getCrystal(color));
			}
			SinglePlayerInfo* player_info = update_info.add_player_infos();
		    player_info->set_id(m_currentPlayerID);
			player_info->set_gem(src->getGem());
			player_info->set_crystal(src->getCrystal());
			engine->sendMessage(-1, MSG_GAME, update_info);
			engine->popGameState();
			engine->pushGameState(new StateAfterSpecial(m_currentPlayerID));
			engine->pushGameState(new StateBeforeSpecial(m_currentPlayerID));
			return GE_SUCCESS;
		}
		break;
	default:
		return GE_INVALID_ACTION;
	}
}

int StateActionPhase::magicSkill(Action *action, GameGrail* engine)
{
	int ret;
	int m_currentPlayerID = engine->getCurrentPlayerID();
	PlayerEntity *src = engine->getPlayerEntity(m_currentPlayerID);
	if(GE_SUCCESS == (ret = src->v_magic_skill(action))){
		engine->popGameState();
		engine->pushGameState(new StateAfterMagic(m_currentPlayerID));
		engine->pushGameState(new StateMagicSkill(action));
		engine->pushGameState(new StateBeforeMagic(m_currentPlayerID));
		return GE_SUCCESS;
	}
	return ret;
}

int StateActionPhase::attackSkill(Action *action, GameGrail* engine)
{
	int ret;
	int m_currentPlayerID = engine->getCurrentPlayerID();
	PlayerEntity *src = engine->getPlayerEntity(m_currentPlayerID);
	if(GE_SUCCESS == (ret = src->v_attack_skill(action))){
		engine->popGameState();
		engine->pushGameState(new StateAfterAttack(m_currentPlayerID));
		engine->pushGameState(new StateAttackSkill(action));
		engine->pushGameState(new StateBeforeAttack(action->dst_ids(0), m_currentPlayerID));
		return GE_SUCCESS;
	}
	return ret;
}

int StateActionPhase::specialSkill(Action *action, GameGrail* engine)
{
	int ret;
	int m_currentPlayerID = engine->getCurrentPlayerID();
	PlayerEntity *src = engine->getPlayerEntity(m_currentPlayerID);
	if(GE_SUCCESS == (ret = src->v_special_skill(action))){
		engine->popGameState();
		engine->pushGameState(new StateAfterSpecial(m_currentPlayerID));
		engine->pushGameState(new StateSpecialSkill(action));
		engine->pushGameState(new StateBeforeSpecial(m_currentPlayerID));
		return GE_SUCCESS;
	}
	return ret;
}

int StateBeforeAttack::handle(GameGrail* engine)
{
	int ret = GE_FATAL_ERROR;
	int m_currentPlayerID = engine->getCurrentPlayerID();
	PlayerEntity *handler = NULL;
	while (iterator < engine->getGameMaxPlayers()) {
		handler = engine->getNextPlayerEntity(handler, iterator, step);
		ret = handler->p_before_attack(step, dstID, srcID);
		moveIterator(ret);
		if(GE_SUCCESS != ret){
			return ret;
		}		
	}
	return engine->popGameState_if(STATE_BEFORE_ATTACK);
}

int StateAttacked::handle(GameGrail* engine)
{
	if(RATE_NOMISS == context->hitRate){
		CONTEXT_TIMELINE_1 temp = *context;
		engine->popGameState();
		return engine->setStateTimeline2Hit(temp.attack.cardID, temp.attack.dstID, temp.attack.srcID, temp.harm, temp.attack.isActive);
	}
	CommandRequest cmd_req;
	Coder::askForReBat(context->hitRate, context->attack.cardID, context->attack.dstID, context->attack.srcID, cmd_req);

	if(engine->waitForOne(context->attack.dstID, MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		int ret;

		CONTEXT_TIMELINE_1 temp = *context;
		if(GE_SUCCESS == (ret = engine->getReply(context->attack.dstID, reply))){
			Respond *reply_attack = (Respond*) reply;

			int ra = reply_attack->args(0);
			int card_id;

			switch(ra)
			{
				//FIXME: verify
			case RA_ATTACK:
				card_id = reply_attack->card_ids(0);
				if(GE_SUCCESS == (ret=engine->getPlayerEntity(context->attack.dstID)->v_reattack(card_id, temp.attack.cardID, reply_attack->dst_ids().Get(0), temp.attack.srcID, context->hitRate))){
					// 反馈玩家行动
					reply_attack->set_src_id(context->attack.dstID);					
					
					bool realCard = true;
					int realCardID = card_id;
					PlayerEntity *handler = NULL;
					while (iterator < engine->getGameMaxPlayers()) {
						handler = engine->getNextPlayerEntity(handler, iterator, step);
						ret = handler->p_reattack(step, card_id, temp.attack.dstID, reply_attack->dst_ids().Get(0), realCard);
						moveIterator(ret);
					}
					engine->popGameState();
					engine->setStateReattack(temp.attack.cardID, card_id, temp.attack.srcID, temp.attack.dstID, reply_attack->dst_ids().Get(0), temp.attack.isActive, realCard);
					if(!realCard)
					{
						CardMsg show_card;
						Coder::showCardNotice(temp.attack.dstID, 1, realCardID, show_card);
						engine->sendMessage(-1, MSG_CARD, show_card);
						engine->setStateMoveOneCardNotToHand(temp.attack.dstID, DECK_HAND, -1, DECK_DISCARD, realCardID, temp.attack.dstID, CAUSE_USE, true);
					}
					return GE_URGENT;
				}
				break;
			case RA_BLOCK:
				card_id = reply_attack->card_ids(0);
				if(GE_SUCCESS == (ret=engine->getPlayerEntity(context->attack.dstID)->v_block(card_id))){
					// 反馈玩家行动
					reply_attack->set_src_id(context->attack.dstID);

					engine->popGameState();
					engine->setStateTimeline2Miss(temp.attack.cardID, temp.attack.dstID, temp.attack.srcID, temp.attack.isActive);
					return engine->setStateUseCard(card_id, temp.attack.srcID, temp.attack.dstID);				
				}
				break;
			case RA_GIVEUP:
				engine->popGameState();
				return engine->setStateAttackGiveUp(temp.attack.cardID, temp.attack.dstID, temp.attack.srcID, temp.harm, temp.attack.isActive, temp.checkShield);
				break;
			}
		}
		return ret;
	}
	else{
		CONTEXT_TIMELINE_1 temp = *context;
		engine->popGameState();
		engine->setStateAttackGiveUp(temp.attack.cardID, temp.attack.dstID, temp.attack.srcID, temp.harm, temp.attack.isActive, temp.checkShield);
		return GE_TIMEOUT;
	}
}

int StateAttackSkill::handle(GameGrail* engine)
{
	int ret = GE_FATAL_ERROR;
	int m_currentPlayerID = engine->getCurrentPlayerID();
	PlayerEntity *src = engine->getPlayerEntity(m_currentPlayerID);
	if(step != STEP_DONE){
		ret = src->p_attack_skill(step, action);
		if(GE_SUCCESS != ret){
			return ret;		
		}
	}
	return engine->popGameState_if(STATE_ATTACK_SKILL);
}

int StateAfterAttack::handle(GameGrail* engine)
{
	int ret = GE_FATAL_ERROR;
	int m_currentPlayerID = engine->getCurrentPlayerID();
	PlayerEntity *handler = NULL;
	while (iterator < engine->getGameMaxPlayers()) {
		handler = engine->getNextPlayerEntity(handler, iterator, step);
		ret = handler->p_after_attack(step, srcID);
		moveIterator(ret);
		if(GE_SUCCESS != ret){
			return ret;
		}		
	}
	if(GE_SUCCESS == (ret = engine->popGameState_if(STATE_AFTER_ATTACK))){
		return engine->setStateCheckTurnEnd();
	}
	return ret;
}

int StateBeforeMagic::handle(GameGrail* engine)
{
	int ret = GE_FATAL_ERROR;
	PlayerEntity *handler = NULL;
	while (iterator < engine->getGameMaxPlayers()) {
		handler = engine->getNextPlayerEntity(handler, iterator, step);
		ret = handler->p_before_magic(step, srcID);
		moveIterator(ret);
		if(GE_SUCCESS != ret){
			return ret;
		}		
	}
	return engine->popGameState_if(STATE_BEFORE_MAGIC);
}

StateMissiled* StateMissiled::create(GameGrail* engine, int cardID, int dstID, int srcID)
{
	int ret;
	PlayerEntity* src = engine->getPlayerEntity(srcID);
	if(GE_SUCCESS != (ret = src->v_missile(cardID, dstID))){
		throw (GrailError)ret;
	}
	StateMissiled* probe = new StateMissiled(dstID, srcID, false);
	if(dstID == probe->getNextTargetID(engine, srcID)){
		return probe;
	}
	else{
		probe->isClockwise = true;
		return probe;
	}
}

int StateMissiled::handle(GameGrail* engine)
{
	int nextTargetID = getNextTargetID(engine, dstID);

	CommandRequest cmd_req;
	Coder::askForMissile(dstID, srcID, harmPoint, nextTargetID, cmd_req);
	int dstID_t = dstID;
	int srcID_t = srcID;
	int harmPoint_t = harmPoint;
//change
	if(engine->waitForOne(dstID, MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		int ret;
		Respond* respond;
		if(GE_SUCCESS == (ret = engine->getReply(dstID, reply)))
		{
			respond = (Respond*)reply;
			int cardID;			
			switch(respond->args(0))
			{				
			case RA_ATTACK:
				cardID = respond->card_ids(0);
				if(GE_SUCCESS == (ret=engine->getPlayerEntity(dstID)->v_remissile(cardID))){
					srcID = dstID;
					dstID = nextTargetID;
					harmPoint++;
					hasMissiled[srcID] = true;
					return engine->setStateUseCard(cardID, dstID, srcID);
				}
				break;
			case RA_BLOCK:
				cardID = respond->card_ids(0);
				if(GE_SUCCESS == (ret=engine->getPlayerEntity(dstID)->v_block(cardID))){
					engine->popGameState();
					return engine->setStateUseCard(cardID, srcID_t, dstID_t);				
				}
				break;
			case RA_GIVEUP:				
				int harmPoint_t = harmPoint;
				engine->popGameState();
				return engine->setStateMissileGiveUp(dstID_t, srcID_t, harmPoint_t);
				break;
			}
		}
		return ret;
	}
	else{
		engine->popGameState();
		engine->setStateMissileGiveUp(dstID_t, srcID_t, harmPoint_t);
		return GE_TIMEOUT;
	}
}

int StateMissiled::getNextTargetID(GameGrail* engine ,int startID)
{
	int nextTargetID;
	PlayerEntity* start = engine->getPlayerEntity(startID);
	int color = start->getColor();
	PlayerEntity* it = start;
	while(true)
	{
		while(start != (it = isClockwise ? it->getPre() : it->getPost()))
		{
			nextTargetID = it->getID();
			if(!hasMissiled[nextTargetID] && it->getColor() != color){								
				return nextTargetID;
			}
		}
		memset(hasMissiled, 0, sizeof(hasMissiled));
	}
}

int StateMagicSkill::handle(GameGrail* engine)
{
	int ret = GE_FATAL_ERROR;
	int m_currentPlayerID = engine->getCurrentPlayerID();
	PlayerEntity *src = engine->getPlayerEntity(m_currentPlayerID);
	if(step != STEP_DONE){
		ret = src->p_magic_skill(step, action);
		if(GE_SUCCESS != ret){
			return ret;		
		}
	}
	return engine->popGameState_if(STATE_MAGIC_SKILL);
}

int StateAfterMagic::handle(GameGrail* engine)
{
	int ret = GE_FATAL_ERROR;
	PlayerEntity *handler = NULL;
	while (iterator < engine->getGameMaxPlayers()) {
		handler = engine->getNextPlayerEntity(handler, iterator, step);
		ret = handler->p_after_magic(step, srcID);
		moveIterator(ret);
		if(GE_SUCCESS != ret){
			return ret;
		}		
	}
	if(GE_SUCCESS == (ret = engine->popGameState_if(STATE_AFTER_MAGIC))){
		return engine->setStateCheckTurnEnd();
	}
	return ret;
}

int StateBeforeSpecial::handle(GameGrail* engine)
{
	int ret = GE_FATAL_ERROR;
	PlayerEntity *handler = NULL;
	while (iterator < engine->getGameMaxPlayers()) {
		handler = engine->getNextPlayerEntity(handler, iterator, step);
		ret = handler->p_before_special(step, srcID);
		moveIterator(ret);
		if(GE_SUCCESS != ret){
			return ret;
		}		
	}
	return engine->popGameState_if(STATE_BEFORE_SPECIAL);
}

int StateSpecialSkill::handle(GameGrail* engine)
{
	int ret = GE_FATAL_ERROR;
	int m_currentPlayerID = engine->getCurrentPlayerID();
	PlayerEntity *src = engine->getPlayerEntity(m_currentPlayerID);
	if(step != STEP_DONE){
		ret = src->p_special_skill(step, action);
		if(GE_SUCCESS != ret){
			return ret;		
		}
	}
	return engine->popGameState_if(STATE_SPECIAL_SKILL);
}

int StateAfterSpecial::handle(GameGrail* engine)
{
	int ret = GE_FATAL_ERROR;
	PlayerEntity *handler = NULL;
	while (iterator < engine->getGameMaxPlayers()) {
		handler = engine->getNextPlayerEntity(handler, iterator, step);
		ret = handler->p_after_special(step, srcID);
		moveIterator(ret);
		if(GE_SUCCESS != ret){
			return ret;
		}		
	}
	if(GE_SUCCESS == (ret = engine->popGameState_if(STATE_AFTER_SPECIAL))){
		return engine->setStateCheckTurnEnd();
	}
	return ret;
}

int StateAdditionalAction::handle(GameGrail* engine)
{
	int ret = GE_FATAL_ERROR;
	int m_currentPlayerID = engine->getCurrentPlayerID();
	PlayerEntity *dst = engine->getPlayerEntity(m_currentPlayerID);
	list<ACTION_QUOTA> quota = dst->getAdditionalAction();

	CommandRequest cmd_req;
	Coder::askForAdditionalAction(m_currentPlayerID, quota, cmd_req);

	if(engine->waitForOne(m_currentPlayerID, MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		int ret;
		if(GE_SUCCESS == (ret=engine->getReply(m_currentPlayerID, reply)))
		{
			Respond *respond = (Respond*)reply;
			int chosen = respond->args(0);
			//放弃所有行动
			if(chosen == ACTION_NONE){
				engine->popGameState();
				engine->pushGameState(new StateTurnEnd);
			}
			else if(GE_SUCCESS == dst->v_additional_action(chosen)){
				return dst->p_additional_action(chosen);
			}
		}
		return ret;
	}
	else 
	{
		engine->popGameState();
		engine->pushGameState(new StateTurnEnd);
		return GE_TIMEOUT;
	}
}

int StateTurnEnd::handle(GameGrail* engine)
{
	int ret = GE_FATAL_ERROR;
	int m_currentPlayerID = engine->getCurrentPlayerID();
	PlayerEntity *handler = NULL;
	while (iterator < engine->getGameMaxPlayers()) {
		handler = engine->getNextPlayerEntity(handler, iterator, step);
		ret = handler->p_turn_end(step, m_currentPlayerID);
		moveIterator(ret);
		if(GE_SUCCESS != ret){
			return ret;
		}		
	}

	if(GE_SUCCESS == (ret = engine->popGameState_if(STATE_TURN_END))){
		PlayerEntity* player = engine->getPlayerEntity(m_currentPlayerID);
		return engine->setStateCurrentPlayer(player->getPost()->getID());
	}
	return ret;	
}

int StateTimeline1::handle(GameGrail* engine)
{
	int ret = GE_FATAL_ERROR;
	PlayerEntity *handler = NULL;
	while (iterator < engine->getGameMaxPlayers()) {
		handler = engine->getNextPlayerEntity(handler, iterator, step);
		ret = handler->p_timeline_1(step, context);
		moveIterator(ret);
		if(GE_SUCCESS != ret){
			return ret;
		}		
	}

	CONTEXT_TIMELINE_1* con = new CONTEXT_TIMELINE_1;
	*con = *context;
	if(GE_SUCCESS == (ret = engine->popGameState_if(STATE_TIMELINE_1))){		
		engine->pushGameState(new StateAttacked(con));
	}
	else{
		SAFE_DELETE(con);
	}
	return ret;
}

int StateTimeline2Hit::handle(GameGrail* engine)
{
	int ret = GE_FATAL_ERROR;
	PlayerEntity *handler = NULL;
	while (iterator < engine->getGameMaxPlayers()) {
		handler = engine->getNextPlayerEntity(handler, iterator, step);
		ret = handler->p_timeline_2_hit(step, context);
		moveIterator(ret);
		if(GE_SUCCESS != ret){
			return ret;
		}		
	}

	CONTEXT_TIMELINE_2_HIT temp = *context;
	if(GE_SUCCESS == (ret = engine->popGameState_if(STATE_TIMELINE_2_HIT))){
		return engine->setStateTimeline3(temp.attack.dstID,temp.harm);
	}
	return ret;
}

int StateTimeline2Miss::handle(GameGrail* engine)
{
	int ret = GE_FATAL_ERROR;
	PlayerEntity *handler = NULL;
	while (iterator < engine->getGameMaxPlayers()) {
		handler = engine->getNextPlayerEntity(handler, iterator, step);
		ret = handler->p_timeline_2_miss(step, context);
		moveIterator(ret);
		if(GE_SUCCESS != ret){
			return ret;
		}		
	}
	return engine->popGameState_if(STATE_TIMELINE_2_MISS);
}

int StateHarmEnd::handle(GameGrail* engine)
{
	int ret = GE_FATAL_ERROR;
	PlayerEntity *handler = NULL;
	while (iterator < engine->getGameMaxPlayers()) {
		handler = engine->getNextPlayerEntity(handler, iterator, step);
		ret = handler->p_harm_end(step, context);
		moveIterator(ret);
		if(GE_SUCCESS != ret){
			return ret;
		}		
	}
	return engine->popGameState_if(STATE_HARM_END);
}

int StateTimeline3::handle(GameGrail* engine)
{
	int ret = GE_FATAL_ERROR;
	if(!isSet){
		network::HurtMsg hurt_msg;
		HARM harm = context->harm;
		Coder::hurtNotice(context->dstID, harm.srcID, harm.type, harm.point, harm.cause, hurt_msg);
		engine->sendMessage(-1, MSG_HURT, hurt_msg);
		isSet = true;
	}
	PlayerEntity *handler = NULL;
	while (iterator < engine->getGameMaxPlayers()) {
		handler = engine->getNextPlayerEntity(handler, iterator, step);
		ret = handler->p_timeline_3(step, context);
		moveIterator(ret);
		if(GE_SUCCESS != ret){
			return ret;
		}		
	}

	CONTEXT_HARM_END* endCon = new CONTEXT_HARM_END;
	endCon->harm = context->harm;
	endCon->dstID = context->dstID;

	PlayerEntity* player = engine->getPlayerEntity(context->dstID);
	int cross = player->getCrossNum();
	if(cross > 0){
		CONTEXT_TIMELINE_4* newCon = new CONTEXT_TIMELINE_4;
		newCon->harm = context->harm;
		newCon->dstID = context->dstID;
		newCon->crossAvailable = cross;
		if(GE_SUCCESS == (ret = engine->popGameState_if(STATE_TIMELINE_3))){
			engine->pushGameState(new StateHarmEnd(endCon));
			engine->pushGameState(new StateTimeline4(newCon));
		}
		else{
			SAFE_DELETE(endCon);
			SAFE_DELETE(newCon);
		}
	}
	else{
		CONTEXT_TIMELINE_5* newCon = new CONTEXT_TIMELINE_5;
		newCon->dstID = context->dstID;
		newCon->harm = context->harm;
		if(GE_SUCCESS == (ret = engine->popGameState_if(STATE_TIMELINE_3))){
			engine->pushGameState(new StateHarmEnd(endCon));
			engine->pushGameState(new StateTimeline5(newCon));
		}
		else{
			SAFE_DELETE(endCon);
			SAFE_DELETE(newCon);
		}
	}
	return ret;
}

int StateTimeline4::handle(GameGrail* engine)
{
	int ret = GE_FATAL_ERROR;
	PlayerEntity *handler = NULL;
	while (iterator < engine->getGameMaxPlayers()) {
		handler = engine->getNextPlayerEntity(handler, iterator, step);
		ret = handler->p_timeline_4(step, context);
		moveIterator(ret);
		if(GE_SUCCESS != ret){
			return ret;
		}		
	}

	if(context->crossAvailable>0){
		CONTEXT_TIMELINE_4 temp = *context;
		if(GE_SUCCESS == (ret = engine->popGameState_if(STATE_TIMELINE_4))){
			engine->pushGameState(new StateAskForCross(temp.dstID, temp.harm, temp.crossAvailable));
		}		
	}
	else{
		CONTEXT_TIMELINE_5* newCon = new CONTEXT_TIMELINE_5;
		newCon->dstID = context->dstID;
		newCon->harm = context->harm;
		if(GE_SUCCESS == (ret = engine->popGameState_if(STATE_TIMELINE_4))){
			engine->pushGameState(new StateTimeline5(newCon));
		}
		else{
			SAFE_DELETE(newCon);
		}
	}
	return ret;
}

int StateTimeline5::handle(GameGrail* engine)
{
	int ret = GE_FATAL_ERROR;
	PlayerEntity *handler = NULL;
	while (iterator < engine->getGameMaxPlayers()) {
		handler = engine->getNextPlayerEntity(handler, iterator, step);
		ret = handler->p_timeline_5(step, context);
		moveIterator(ret);
		if(GE_SUCCESS != ret){
			return ret;
		}		
	}

	if(context->harm.point>0){
		CONTEXT_TIMELINE_6 *newCon = new CONTEXT_TIMELINE_6;
		newCon->dstID = context->dstID;
		newCon->harm = context->harm;
		if(GE_SUCCESS == (ret = engine->popGameState_if(STATE_TIMELINE_5))){
			engine->pushGameState(new StateTimeline6(newCon));
		}
		else{
			SAFE_DELETE(newCon);
		}
	}
	else{
		ret = engine->popGameState_if(STATE_TIMELINE_5);
	}
	return ret;
}

int StateTimeline6::handle(GameGrail* engine)
{
	int ret = GE_FATAL_ERROR;
	PlayerEntity *handler = NULL;
	while (iterator < engine->getGameMaxPlayers()) {
		handler = engine->getNextPlayerEntity(handler, iterator, step);
		ret = handler->p_timeline_6(step, context);
		moveIterator(ret);
		if(GE_SUCCESS != ret){
			return ret;
		}		
	}

	CONTEXT_TIMELINE_6 temp = *context;
	vector<int> cards;
	if(context->harm.point>0){
		CONTEXT_TIMELINE_6_DRAWN *newCon = new CONTEXT_TIMELINE_6_DRAWN;
		newCon->dstID = context->dstID;
		newCon->harm = context->harm;
		if(GE_SUCCESS == (ret = engine->popGameState_if(STATE_TIMELINE_6))){
			engine->pushGameState(new StateTimeline6Drawn(newCon));
			ret = engine->setStateMoveCardsToHand(-1, DECK_PILE, temp.dstID, DECK_HAND, temp.harm.point, cards, temp.harm, false);
		}
		else{
			SAFE_DELETE(newCon);
		}
	}
	else{
		ret = engine->popGameState_if(STATE_TIMELINE_6);
	}
	return ret;
}

int StateTimeline6Drawn::handle(GameGrail* engine)
{
	int ret = GE_FATAL_ERROR;
	PlayerEntity *handler = NULL;
	while (iterator < engine->getGameMaxPlayers()) {
		handler = engine->getNextPlayerEntity(handler, iterator, step);
		ret = handler->p_timeline_6_drawn(step, context);
		moveIterator(ret);
		if(GE_SUCCESS != ret){
			return ret;
		}		
	}

	ret = engine->popGameState_if(STATE_TIMELINE_6_DRAWN);
	return ret;
}

int StateAskForCross::handle(GameGrail* engine)
{
	CommandRequest cmd_req;
	Coder::askForCross(dstID, harm.point, harm.type, crossAvailable, cmd_req);
	int ret;
	if(engine->waitForOne(dstID, MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(dstID, reply)))
		{
			Respond* reCross = (Respond*) reply;
			int usedCross = reCross->args(0);
			if(usedCross>0)
			{
				PlayerEntity *dstPlayer = engine->getPlayerEntity(dstID);
				dstPlayer->subCrossNum(usedCross);
				GameInfo update_info;
				Coder::crossNotice(dstID, dstPlayer->getCrossNum(), update_info);
				engine->sendMessage(-1, MSG_GAME, update_info);
				CONTEXT_TIMELINE_5* newCon = new CONTEXT_TIMELINE_5;
				newCon->dstID = dstID;
				HARM newHarm = harm;
				newHarm.point -= usedCross;
				newCon->dstID = dstID;
				newCon->harm = newHarm;
				engine->popGameState();
				engine->pushGameState(new StateTimeline5(newCon));
				return ret;
			}
			else
			{
				CONTEXT_TIMELINE_5* newCon = new CONTEXT_TIMELINE_5;
				newCon->dstID = dstID;
				newCon->harm = harm;
				engine->popGameState();
				engine->pushGameState(new StateTimeline5(newCon));
				return ret;
			}
		}
		else
			return ret;
	}
	else
	{
		//Timeout, skip to nextState
		CONTEXT_TIMELINE_5* newCon = new CONTEXT_TIMELINE_5;
		newCon->dstID = dstID;
		newCon->harm = harm;
		engine->popGameState();
		engine->pushGameState(new StateTimeline5(newCon));
		return GE_TIMEOUT;
	}
}

int StateHandChange::handle(GameGrail* engine)
{
	int ret;
	if(!isSet){
		PlayerEntity* dst = engine->getPlayerEntity(dstID);
		if(direction == CHANGE_ADD){
			dst->addHandCards(howMany, cards);					
		}
		else{
			dst->removeHandCards(howMany, cards);
		}
		isSet = true;

		GameInfo update_info;
		Coder::handNotice(dstID, dst->getHandCards(), update_info);
	
		engine->sendMessage(-1, MSG_GAME, update_info);
	}
	ret = GE_FATAL_ERROR;
	PlayerEntity *handler = NULL;
	while (iterator < engine->getGameMaxPlayers()) {
		handler = engine->getNextPlayerEntity(handler, iterator, step);
		ret = handler->p_hand_change(step, dstID);
		moveIterator(ret);
		if(GE_SUCCESS != ret){
			return ret;
		}		
	}

	int dstID_temp = dstID;
	HARM harm_temp = harm;
	if(GE_SUCCESS == (ret = engine->popGameState_if(STATE_HAND_CHANGE))){
		ret = engine->setStateHandOverLoad(dstID_temp, harm_temp);
	}
	return ret;
}

int StateBasicEffectChange::handle(GameGrail* engine)
{
	int ret = GE_SUCCESS;
	if(!isSet){
		PlayerEntity* dst = engine->getPlayerEntity(dstID);
		if(direction == CHANGE_ADD){
            ret = dst->addBasicEffect(card, doerID);
		}
		else{
			ret = dst->removeBasicEffect(card);	
		}
		isSet = true;

		GameInfo update_info;
		Coder::basicNotice(dstID, dst->getBasicEffect(), update_info);
		engine->sendMessage(-1, MSG_GAME, update_info);

		// 没有移除或者没有添加到基础效果区
		if (ret == GE_MOVECARD_FAILED)
		{
			engine->popGameState_if(STATE_BASIC_EFFECT_CHANGE);
			return ret;
		}
	}

	PlayerEntity *handler = NULL;
	while (iterator < engine->getGameMaxPlayers()) {
		handler = engine->getNextPlayerEntity(handler, iterator, step);
		ret = handler->p_basic_effect_change(step, dstID, card, doerID, cause);
		moveIterator(ret);
		if(GE_SUCCESS != ret){
			return ret;
		}		
	}
	return engine->popGameState_if(STATE_BASIC_EFFECT_CHANGE);
}

int StateCoverChange::handle(GameGrail* engine)
{
	int ret;
	if(!isSet){
		PlayerEntity* dst = engine->getPlayerEntity(dstID);
		if(direction == CHANGE_ADD){
			dst->addCoverCards(howMany, cards);					
		}
		else{
			dst->removeCoverCards(howMany, cards);
		}
		isSet = true;

		GameInfo update_info;
		Coder::coverNotice(dstID, dst->getCoverCards(), update_info);
	
		engine->sendMessage(-1, MSG_GAME, update_info);
	}
	ret = GE_FATAL_ERROR;
	PlayerEntity *handler = NULL;
	while (iterator < engine->getGameMaxPlayers()) {
		handler = engine->getNextPlayerEntity(handler, iterator, step);
		ret = handler->p_cover_change(step, dstID, direction, howMany, cards, harm.srcID, harm.cause);
		moveIterator(ret);
		if(GE_SUCCESS != ret){
			return ret;
		}		
	}

	int dstID_temp = dstID;
	if(GE_SUCCESS == (ret = engine->popGameState_if(STATE_COVER_CHANGE))){
		ret = engine->setStateCoverOverLoad(dstID_temp);
	}
	return ret;
}

int StateRequestHand::handle(GameGrail* engine)
{
	int ret = GE_FATAL_ERROR;
	//最多把手牌全弃，若手牌为零，直接pop
	int atMost = engine->getPlayerEntity(targetID)->getHandCardNum();
	harm.point = harm.point > atMost ? atMost : harm.point;
	int targetID_t = targetID;
	int dstOwner_t = dstOwner;
	int dstArea_t = dstArea;
	HARM harm_t = harm;
	bool isShown_t = isShown;
	vector<int> toDiscard(harm.point);
	PlayerEntity *target = engine->getPlayerEntity(targetID);
	PlayerEntity *causer = engine->getPlayerEntity(harm.srcID);

	if(harm.point < 1){
		engine->popGameState();
		return causer->p_request_hand_give_up(step, targetID_t, harm_t.cause);		 
	}

	CommandRequest cmd_req;
	Coder::askForDiscard(targetID, harm.point, harm.cause, isShown, cmd_req);

	if(engine->waitForOne(targetID, MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(targetID, reply)))
		{
			Respond* respond = (Respond*) reply;
			//弃牌了
			if(respond->args(0) == 1)
			{
				if (respond->card_ids_size() != harm_t.point){
					return GE_MOVECARD_FAILED;
				}
				else {
					for (int i=0; i<respond->card_ids_size(); ++i)
						toDiscard[i] = respond->card_ids(i);
				}
				if(GE_SUCCESS != (ret = target->checkHandCards(harm.point, toDiscard)) ||
				   GE_SUCCESS != (ret = causer->v_request_hand(targetID, harm.point, toDiscard, harm))){
					return ret;
				}
				if(isShown){
					CardMsg show_card;
					Coder::showCardNotice(targetID, harm.point, toDiscard, show_card);
					engine->sendMessage(-1, MSG_CARD, show_card);
				}
				engine->popGameState();
				if(dstArea_t != DECK_HAND){
					return engine->setStateMoveCardsNotToHand(targetID_t, DECK_HAND, dstOwner_t, dstArea_t, harm_t.point, toDiscard, harm_t.srcID, harm_t.cause, isShown_t);
				}
				else{
					return engine->setStateMoveCardsToHand(targetID_t, DECK_HAND, dstOwner_t, dstArea_t, harm_t.point, toDiscard, harm_t, isShown_t);
				}
			}
			else if(canGiveUp){	
				engine->popGameState();
				return causer->p_request_hand_give_up(step, targetID_t, harm_t.cause);
			}
			else{
				return GE_INVALID_ACTION;
			}
		}
	    return ret;
	}
	else
	{
		//Timeout auto discard
		if(!canGiveUp)
		{			
			list<int> handcards = engine->getPlayerEntity(targetID)->getHandCards();
			list<int>::iterator it = handcards.begin();
			for(int i = 0; i < harm.point && it != handcards.end(); i++){
				toDiscard[i] = *it;
				it++;
			}
			if(isShown){
				CardMsg show_card;
				Coder::showCardNotice(targetID, harm.point, toDiscard, show_card);
				engine->sendMessage(-1, MSG_CARD, show_card);
			}
			engine->popGameState();
			if(dstArea != DECK_HAND){
				engine->setStateMoveCardsNotToHand(targetID_t, DECK_HAND, dstOwner_t, dstArea_t, harm_t.point, toDiscard, harm_t.srcID, harm_t.cause, isShown_t);
			}
			else{
				engine->setStateMoveCardsToHand(targetID_t, DECK_HAND, dstOwner_t, dstArea_t, harm_t.point, toDiscard, harm_t, isShown_t);
			}
		}
		else{
			engine->popGameState();
			causer->p_request_hand_give_up(step, targetID_t, harm_t.cause);
		}
		return GE_TIMEOUT;
	}
}

int StateRequestCover::handle(GameGrail* engine)
{
	//最多把手牌全弃，若手牌为零，直接pop
	int atMost = engine->getPlayerEntity(targetID)->getCoverCardNum();
	harm.point = harm.point > atMost ? atMost : harm.point;
	if(harm.point < 1){
		engine->popGameState();
		return GE_SUCCESS;
	}
	int ret = GE_FATAL_ERROR;

	CommandRequest cmd_req;
	Coder::askForDiscardCover(targetID, harm.point, harm.cause, cmd_req);

	int targetID_t = targetID;
	int dstOwner_t = dstOwner;
	int dstArea_t = dstArea;
	bool isShown_t = isShown;
	HARM harm_t = harm;
	vector<int> toDiscard(harm.point);
	PlayerEntity *target = engine->getPlayerEntity(targetID);
	PlayerEntity *causer = engine->getPlayerEntity(harm.srcID);
	if(engine->waitForOne(targetID, MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(targetID, reply)))
		{
			Respond* respond = (Respond*) reply;
			//弃牌了
			if(respond->args(0) == 1)
			{
				if (respond->card_ids_size() != harm.point){
					return GE_MOVECARD_FAILED;
				}
				else {
					for (int i=0; i<respond->card_ids_size(); ++i)
						toDiscard[i] = respond->card_ids(i);
				}
				if(GE_SUCCESS != (ret = target->checkCoverCards(harm.point, toDiscard)) ||
				   GE_SUCCESS != (ret = causer->v_request_cover(harm.point, toDiscard, harm))){
					return ret;
				}
				if(isShown){
					CardMsg show_card;
					Coder::showCardNotice(targetID, harm.point, toDiscard, show_card);
					engine->sendMessage(-1, MSG_CARD, show_card);
				}
				engine->popGameState();
				if(dstArea_t != DECK_HAND){
					return engine->setStateMoveCardsNotToHand(targetID_t, DECK_COVER, dstOwner_t, dstArea_t, harm_t.point, toDiscard, harm_t.srcID, harm_t.cause, isShown_t);
				}
				else{
					return engine->setStateMoveCardsToHand(targetID_t, DECK_COVER, dstOwner_t, dstArea_t, harm_t.point, toDiscard, harm_t, isShown_t);
				}
			}
			else if(canGiveUp){	
				engine->popGameState();
				return causer->p_request_cover_give_up(step, targetID_t, harm_t.cause);
			}
			else{
				return GE_INVALID_ACTION;
			}
		}
	    return ret;
	}
	else
	{
		//Timeout auto discard
		if(!canGiveUp)
		{			
			list<int> covercards = engine->getPlayerEntity(targetID)->getCoverCards();
			list<int>::iterator it = covercards.begin();
			for(int i = 0; i < harm.point && it != covercards.end(); i++){
				toDiscard[i] = *it;
				it++;
			}
			if(isShown){
				CardMsg show_card;
				Coder::showCardNotice(targetID, harm.point, toDiscard, show_card);
				engine->sendMessage(-1, MSG_CARD, show_card);
			}
			engine->popGameState();
			if(dstArea != DECK_HAND){
				engine->setStateMoveCardsNotToHand(targetID_t, DECK_COVER, dstOwner_t, dstArea_t, harm_t.point, toDiscard, harm_t.srcID, harm_t.cause, isShown_t);
			}
			else{
				engine->setStateMoveCardsToHand(targetID_t, DECK_COVER, dstOwner_t, dstArea_t, harm_t.point, toDiscard, harm_t, isShown_t);
			}
		}
		else{
			engine->popGameState();
			causer->p_request_cover_give_up(step, targetID_t, harm_t.cause);
		}
		return GE_TIMEOUT;
	}
}

int StateBeforeLoseMorale::handle(GameGrail* engine)
{
	int ret = GE_FATAL_ERROR;
	PlayerEntity *handler = NULL;
	while (iterator < engine->getGameMaxPlayers()) {
		handler = engine->getNextPlayerEntity(handler, iterator, step);
		ret = handler->p_before_lose_morale(step, context);
		moveIterator(ret);
		if(GE_SUCCESS != ret){
			return ret;
		}		
	}

	CONTEXT_LOSE_MORALE* newCon = new CONTEXT_LOSE_MORALE;
	newCon->howMany = context->howMany;
	newCon->dstID = context->dstID;
	newCon->harm = context->harm;
	if(GE_SUCCESS == (ret = engine->popGameState_if(STATE_BEFORE_LOSE_MORALE))){
		engine->pushGameState(new StateLoseMorale(newCon));
	}
	else{
		SAFE_DELETE(newCon);
	}
	return ret;
}

int StateLoseMorale::handle(GameGrail* engine)
{
	int ret = GE_FATAL_ERROR;
	PlayerEntity *handler = NULL;
	while (iterator < engine->getGameMaxPlayers()) {
		handler = engine->getNextPlayerEntity(handler, iterator, step);
		ret = handler->p_lose_morale(step, context);
		moveIterator(ret);
		if(GE_SUCCESS != ret){
			return ret;
		}		
	}

	CONTEXT_LOSE_MORALE* newCon = new CONTEXT_LOSE_MORALE;
	newCon->howMany = context->howMany;
	newCon->dstID = context->dstID;
	newCon->harm = context->harm;
	if(GE_SUCCESS == (ret = engine->popGameState_if(STATE_LOSE_MORALE))){
		engine->pushGameState(new StateFixMorale(newCon));
	}
	else{
		SAFE_DELETE(newCon);
	}
	return ret;
}

int StateFixMorale::handle(GameGrail* engine)
{
	int ret = GE_FATAL_ERROR;
	PlayerEntity *handler = NULL;
	while (iterator < engine->getGameMaxPlayers()) {
		handler = engine->getNextPlayerEntity(handler, iterator, step);
		ret = handler->p_fix_morale(step, context);
		moveIterator(ret);
		if(GE_SUCCESS != ret){
			return ret;
		}		
	}

	CONTEXT_LOSE_MORALE* newCon = new CONTEXT_LOSE_MORALE;
	newCon->howMany = context->howMany;
	newCon->dstID = context->dstID;
	newCon->harm = context->harm;
	if(GE_SUCCESS == (ret = engine->popGameState_if(STATE_FIX_MORALE))){
		engine->pushGameState(new StateTrueLoseMorale(newCon));
	}
	else{
		SAFE_DELETE(newCon);
	}
	
	return ret;
}

int StateTrueLoseMorale::handle(GameGrail* engine)
{
	int ret = GE_FATAL_ERROR;
	PlayerEntity *handler = NULL;
	while (iterator < engine->getGameMaxPlayers()) {
		handler = engine->getNextPlayerEntity(handler, iterator, step);
		ret = handler->p_true_lose_morale(step, context);
		moveIterator(ret);
		if(GE_SUCCESS != ret){
			return ret;
		}		
	}

	int color = engine->getPlayerEntity(context->dstID)->getColor();
	TeamArea *m_teamArea = engine->getTeamArea();
	int morale = m_teamArea->getMorale(color);
	m_teamArea->setMorale(color, morale - context->howMany);
	morale = m_teamArea->getMorale(color);

	// 更新士气
	GameInfo update_info;
	if (color == RED)
		update_info.set_red_morale(morale);
	else
		update_info.set_blue_morale(morale);
	engine->sendMessage(-1, MSG_GAME, update_info);
	if(GE_SUCCESS == (ret = engine->popGameState_if(STATE_TRUE_LOSE_MORALE))){
		if(morale <= 0){
			engine->pushGameState(new StateGameOver(1-color));
		}
	}
	return ret;
}

int StateShowHand::handle(GameGrail* engine)
{
	int ret = GE_FATAL_ERROR;
	PlayerEntity *handler = NULL;
	while (iterator < engine->getGameMaxPlayers()) {
		handler = engine->getNextPlayerEntity(handler, iterator, step);
		ret = handler->p_show_hand(step, dstID, howMany, cards, harm);
		moveIterator(ret);
		if(GE_SUCCESS != ret){
			return ret;
		}		
	}
	return engine->popGameState_if(STATE_SHOW_HAND);
}

int StateGameOver::handle(GameGrail* engine)
{	
	engine->gameover = true;
	Sleep(1000);
	engine->m_tableLog.redScore = engine->getTeamArea()->getMorale(RED);
	engine->m_tableLog.blueScore = engine->getTeamArea()->getMorale(BLUE);
	engine->m_tableLog.redCupNum = engine->getTeamArea()->getCup(RED);
	engine->m_tableLog.blueCupNum = engine->getTeamArea()->getCup(BLUE);
	engine->m_tableLog.winner = this->getcolor(); 
	for (int i = 0; i < engine->m_tableLog.playerNums; i++) //根据人数写入细节数据
	{
		if (engine->m_tableLog.tableDetail[i].team == engine->m_tableLog.winner)
			engine->m_tableLog.tableDetail[i].result = RESULT_WIN;
		else
			engine->m_tableLog.tableDetail[i].result = RESULT_LOSE;
		DBInstance.userAccountDAO->gameComplete(engine->m_tableLog.tableDetail[i].playerID);
	}
	return GE_SUCCESS;
}