#pragma once
#include "..\PlayerEntity.h"

class ShouLing : public PlayerEntity
{
public:
	ShouLing(GameGrail *engine, int id, int color): PlayerEntity(engine, id, color){
		tokenMax[0] = 4;
		tokenMax[1] = 2;
	}
	//�ҳ���ʵ��������Ҫ��p_xxxx�麯������Щp_xxxx�������ڶ�Ӧ��StateXXXXX�ﱻ����
	//ÿ��p_xxxx������в�ֻһ�����ܣ�step��������������Щ��
	int p_timeline_1(int &step, CONTEXT_TIMELINE_1 *con);
	int p_additional_action(int chosen);
	int p_timeline_2_hit(int &setp, CONTEXT_TIMELINE_2_HIT *con);
	int p_timeline_2_miss(int &setp, CONTEXT_TIMELINE_2_MISS *con);
	int p_tap(int &step, CONTEXT_TAP *con);
	int p_timeline_3(int &setp, CONTEXT_TIMELINE_3 *con);
	int p_timeline_4(int &setp, CONTEXT_TIMELINE_4 *con);
	int p_timeline_6_drawn(int &step, CONTEXT_TIMELINE_6_DRAWN *con);
	int	p_turn_end(int &step, int playerID);
	int	p_before_turn_begin(int &step, int playerID);
	int p_after_attack(int &step, int currentPlayerID);
	int p_show_hand(int &step, int playerID, int howMany, vector<int> cards, HARM harm);
	//��ɫ����Ϣ��������UserTask���ã�ֻҪʹ����waitForXXX��������Ҫ��д�������
	//return true ��ʾ������
	int p_boot(int &step, int currentPlayerID);
	bool cmdMsgParse(UserTask* session, uint16_t type, ::google::protobuf::Message *proto);
private:
	//���弼��
	int WuZheCanXin();
	int ShouHunJingJie();
	int ShouFan(CONTEXT_TIMELINE_3 *con);
	int NiFanZhan(CONTEXT_TIMELINE_1 *con);
	int YuHunBoot();
	int NiFanHit(CONTEXT_TIMELINE_2_HIT *con);
	int YiJiAtk(CONTEXT_TIMELINE_1 *con);
	bool used_wuzhe = false;
	bool yijiatk = false;
	bool nifanatk = false;
	int nifanaddharm = 0;
};
