#pragma once
#include "..\PlayerEntity.h"

class LingFu : public PlayerEntity
{
public:
	LingFu(GameGrail *engine, int id, int color): PlayerEntity(engine, id, color){
	    coverCardsMax = 2;
	}
	bool cmdMsgParse(UserTask* session, uint16_t type, ::google::protobuf::Message *proto);
	int v_magic_skill(Action *action);
	int p_magic_skill(int &step, Action* action);
	int p_timeline_2_hit(int &setp, CONTEXT_TIMELINE_2_HIT *con);
	int p_cover_change(int &step, int dstID, int direction, int howMany, vector<int> cards, int doerID, int cause);
private:
	int FengXing(Action *action);
	int LeiMing_Cast(Action *action);
	int LeiMing_Effect(Action *action);
	int NianZhou();
	int LingLiBengJie();
	int BaiGuiYeXing_Cast();
	int BaiGuiYeXing_Expose(int cardID);
	int BaiGuiYeXing_Effect();
	bool using_LeiMing;
	bool using_LingLiBengJie;
	bool isExposed;
	int dstIDs[2];
};