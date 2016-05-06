#include "YongZhe.h"

enum CAUSE{
	TIAO_XIN = 2101,
    JIN_DUAN_ZHI_LI = 2102,
	NU_HOU = 2103,
	MING_JING_ZHI_SHUI = 2104,
	SI_DOU = 2105,
	JING_PI_LI_JIE = 2106,
};

YongZhe::YongZhe()
{
    makeConnection();
setMyRole(this);

    Button *tiaoXin;
    tiaoXin=new Button(3,QStringLiteral("挑衅"));
    buttonArea->addButton(tiaoXin);
    connect(tiaoXin,SIGNAL(buttonSelected(int)),this,SLOT(TiaoXin()));
}

void YongZhe::normal()
{
    Role::normal();
    Player* myself=dataInterface->getMyself();
    //挑衅
    if(myself->getToken(0)>0)
        buttonArea->enable(3);
        foreach(Player* ptr,dataInterface->getPlayerList())
            if(ptr->getSpecial(1)==1){
                buttonArea->disable(3);
                break;
            }
    unactionalCheck();
}

void YongZhe::NuHou()
{
	gui->reset();
    state=NU_HOU;
    tipArea->setMsg(QStringLiteral("是否发动怒吼？"));
    decisionArea->enable(0);
    decisionArea->enable(1);
}

void YongZhe::MingJingZhiShui()
{
	gui->reset();
    state=MING_JING_ZHI_SHUI;
    tipArea->setMsg(QStringLiteral("是否发动明镜止水？"));
    decisionArea->enable(0);
    decisionArea->enable(1);
}

void YongZhe::JinDuanZhiLi()
{
	gui->reset();
    state=JIN_DUAN_ZHI_LI;
    tipArea->setMsg(QStringLiteral("是否发动禁断之力？"));
    decisionArea->enable(0);
    decisionArea->enable(1);
	
}

void YongZhe::SiDou()
{
	gui->reset();
    state=SI_DOU;
    tipArea->setMsg(QStringLiteral("是否发动死斗？"));
    decisionArea->enable(0);
    decisionArea->enable(1);
}

void YongZhe::TiaoXin()
{
    handArea->reset();
    tipArea->reset();
    playerArea->reset();
    state=TIAO_XIN;

    playerArea->enableEnemy();
    playerArea->setQuota(1);

    decisionArea->enable(1);
    decisionArea->disable(0);
}

void YongZhe::onOkClicked()
{
    Role::onOkClicked();
    SafeList<Player*>selectedPlayers;

    selectedPlayers=playerArea->getSelectedPlayers();

    network::Action* action;
    network::Respond* respond;
    try{
    switch(state)
    {

    //挑衅
	case TIAO_XIN:
        action = newAction(ACTION_MAGIC_SKILL,TIAO_XIN);
		action->set_src_id(myID);
        action->add_dst_ids(selectedPlayers[0]->getID());
        action->add_args(1);
		usedMagic=true;
        emit sendCommand(network::MSG_ACTION, action);
        gui->reset();
        break;
    //禁断之力
    case JIN_DUAN_ZHI_LI:
        respond = newRespond(JIN_DUAN_ZHI_LI);
        respond->add_args(1);
		 
        foreach(Card*ptr,dataInterface->getHandCards()){
			respond->add_card_ids(ptr->getID());
		}
        emit sendCommand(network::MSG_RESPOND, respond);
        gui->reset();
        break;
	 //怒火，明镜止水，死斗
    case NU_HOU:
	case MING_JING_ZHI_SHUI:
	case SI_DOU:
        respond = newRespond(state);
        respond->add_args(1);
        emit sendCommand(network::MSG_RESPOND, respond);
        gui->reset();
        break;
    }

    }catch(int error){
        logic->onError(error);
    }
}

void YongZhe::onCancelClicked()
{
    Role::onCancelClicked();
    QString command;
	network::Action* action;
    network::Respond* respond;
    switch(state)
    {

    //禁断之力，怒吼，明镜止水，死斗
	case TIAO_XIN:
		normal();
		break;
    case JIN_DUAN_ZHI_LI:
    case NU_HOU:
	case MING_JING_ZHI_SHUI:
	case SI_DOU:
		respond = newRespond(state);
        respond->add_args(0);
        gui->reset();
		emit sendCommand(network::MSG_RESPOND, respond);
        break;

    }
}
void YongZhe::turnBegin()
{
    Role::turnBegin();
}

void YongZhe::askForSkill(network::Command* cmd)
{
    switch(cmd->respond_id())
    {
		case TIAO_XIN:
			return TiaoXin();
		case JIN_DUAN_ZHI_LI:
			return JinDuanZhiLi();
		case NU_HOU:
			return NuHou();
		case MING_JING_ZHI_SHUI:
			return MingJingZhiShui();
		case SI_DOU:
			return SiDou();
	
		break;
	default:
        Role::askForSkill(cmd);
    }
}

