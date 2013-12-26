#include "GMCommand.h"
#include "map"

typedef void (*PTRFUN)(GameGrail*, PlayerEntity*, vector<string>&); 
map<string, PTRFUN> cmd_mapping;

/*
energy: 设置角色能量，参数有2个：宝石，水晶
*/
void setEnergy(GameGrail* engine, PlayerEntity* player, vector<string>& ss)
{
	player->setGem(atoi(ss[1].c_str()));
	player->setCrystal(stoi(ss[2].c_str()));

	GameInfo game_info;
	Coder::energyNotice(player->getID(), player->getGem(), player->getCrystal(), game_info);
	engine->sendMessage(-1, MSG_GAME, game_info);
}

/*
card: 往手上添加牌，参数数量不定，为添加的牌id
*/
void addCard(GameGrail* engine, PlayerEntity* player, vector<string>& ss)
{
	int howmany;
	vector<int> cards;
	for (int i = 1; i < ss.size(); ++i)
		cards.push_back(atoi(ss[i].c_str()));
	howmany = ss.size() - 1;
	player->addHandCards(howmany, cards);

	GameInfo game_info;
	Coder::handNotice(player->getID(), player->getHandCards(), game_info);
	engine->sendMessage(-1, MSG_GAME, game_info);
}

/*
card: 往盖牌区添加盖牌，参数数量不定，为添加的牌id
*/
void addCoverCard(GameGrail* engine, PlayerEntity* player, vector<string>& ss)
{
	int howmany;
	vector<int> cards;
	for (int i = 1; i < ss.size(); ++i)
		cards.push_back(atoi(ss[i].c_str()));
	howmany = ss.size() - 1;
	player->addCoverCards(howmany, cards);  //

	GameInfo game_info;
	Coder::coverNotice(player->getID(), player->getCoverCards(), game_info);
	engine->sendMessage(-1, MSG_GAME, game_info);
}

/*
cross: 设置治疗
*/
void setCross(GameGrail* engine, PlayerEntity* player, vector<string>& ss)
{
	int howmany = atoi(ss[1].c_str());
	player->addCrossNum(howmany-player->getCrossNum(), -2);   // 1000以内随便加

	GameInfo game_info;
	Coder::crossNotice(player->getID(), player->getCrossNum(), game_info);
	engine->sendMessage(-1, MSG_GAME, game_info);
}

/*
录入gm指令，将gm指令放入cmd_mapping中，key为gm指令格式的字符串，value是处理函数的指针
*/
void initialize_gm_command()
{
	cmd_mapping["!`energy"] = setEnergy;            // 设置能量
	cmd_mapping["!`card"] = addCard;                // 添加手牌
	cmd_mapping["!`cross"] = setCross;              // 添加治疗
	cmd_mapping["!`covercard"] = addCoverCard;      // 添加盖牌
}

/*
用于指令字符串切割
*/
void split(const string& src, const string& separator, vector<string>& dest)
{
    string str = src;
    string substring;
    string::size_type start = 0, index;

    do
    {
        index = str.find_first_of(separator,start);
        if (index != string::npos)
        {    
            substring = str.substr(start,index-start);
            dest.push_back(substring);
            start = str.find_first_not_of(separator,index);
            if (start == string::npos) return;
        }
    }while(index != string::npos);
    
    //the last token
    substring = str.substr(start);
    dest.push_back(substring);
}

void gm_cmd(GameGrail* engine, PlayerEntity* player, string cmd)
{
	vector<string> ss;
	split(cmd, " ", ss);
	// 根据指令调用对应的gm函数，没有对应指令就直接跳过
	if (cmd_mapping.find(ss[0]) == cmd_mapping.end())
	{
		return;
	}
	PTRFUN func = cmd_mapping[ss[0]];
	func(engine, player, ss);
}
