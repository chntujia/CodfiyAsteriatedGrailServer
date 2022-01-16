#pragma once
#include "..\PlayerEntity.h"

class XueJian : public PlayerEntity
{
public:
	XueJian(GameGrail *engine, int id, int color): PlayerEntity(engine, id, color){ tokenMax[0] = 3; }
	//�ҳ���ʵ��������Ҫ��p_xxxx�麯������Щp_xxxx�������ڶ�Ӧ��StateXXXXX�ﱻ����
	//ÿ��p_xxxx������в�ֻһ�����ܣ�step��������������Щ��
	bool cmdMsgParse(UserTask* session, uint16_t type, ::google::protobuf::Message *proto);
	int p_timeline_2_hit(int &setp, CONTEXT_TIMELINE_2_HIT *con);
	int p_after_attack(int &step, int playerID);
	int p_additional_action(int chosen);
	int v_magic_skill(Action *action);
	int p_magic_skill(int &step, Action* action);
	int p_timeline_3(int &step, CONTEXT_TIMELINE_3 *con);
	int p_timeline_4(int &step, CONTEXT_TIMELINE_4 *con);
	int p_boot(int &step, int currentPlayerID);
	int p_turn_end(int &step, int playerID);
	//��ɫ����Ϣ��������UserTask���ã�ֻҪʹ����waitForXXX��������Ҫ��д�������
	//return true ��ʾ������

private:
	//���弼��
	int XueSeJingJi(CONTEXT_TIMELINE_2_HIT *con);
	int ChiSeYiShan(int playerID);
	int XueRanQiangWei(int &step, Action* action);
	int XueQiPingZhang(CONTEXT_TIMELINE_3 *con);
	int XueQiangWeiBoot();
};
