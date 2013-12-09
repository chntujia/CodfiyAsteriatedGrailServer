#pragma once
#include "..\PlayerEntity.h"

class  LingHun: public PlayerEntity
{
public:
	  LingHun(GameGrail *engine, int id, int color);
bool  cmdMsgParse(UserTask *session, uint16_t type, ::google::protobuf::Message *proto);
int   p_boot(int &step, int currentPlayerID);
int   p_timeline_1(int &step, CONTEXT_TIMELINE_1 *con);
int   p_timeline_6(int &step, CONTEXT_TIMELINE_6 *con);
int   p_timeline_6_drawn(int &step, CONTEXT_TIMELINE_6_DRAWN *con); 
int   p_true_lose_morale(int &step, CONTEXT_LOSE_MORALE *con);
int   v_magic_skill(Action *action);
int   p_magic_skill(int &step, Action* action);

private:
	//具体技能
  int LingHunZengFu();
  int LingHunCiYu(Action* action);
  int LingHunZhenBao(Action *action);
  int LingHunJingXiang(Action* action);
  int LingHunZhaoHuan(Action* action);
  int LingHunZhuanHuan(CONTEXT_TIMELINE_1 *con);
  int LingHunLianJie();
  int LingHunLianJieReact(CONTEXT_TIMELINE_6 *con);
  
  bool used_LING_HUN_LIAN_JIE;
  bool using_LING_HUN_LIAN_JIE;
  int  connectID;

};