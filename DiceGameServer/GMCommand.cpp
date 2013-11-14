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
录入gm指令，将gm指令放入cmd_mapping中，key为gm指令格式的字符串，value是处理函数的指针
*/
void initialize_gm_command()
{
	cmd_mapping["!`energy"] = setEnergy;  // 设置能量
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
