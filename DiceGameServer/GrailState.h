#pragma once
#include "GameGrailCommon.h"
// game state define
class GameGrail;

enum STATE{
	STATE_FATAL_ERROR,
	STATE_WAIT_FOR_ENTER,
	STATE_SEAT_ARRANGE,	
	STATE_ROLE_STRATEGY_RANDOM,
	STATE_ROLE_STRATEGY_31,
	STATE_ROLE_STRATEGY_BP,
	STATE_GAME_START,
	STATE_BEFORE_TURN_BEGIN,
	STATE_TURN_BEGIN,
	STATE_WEAKEN,
	STATE_BEFORE_ACTION,
	STATE_BOOT,	
	STATE_ACTION_PHASE,
	STATE_BEFORE_ATTACK,
	STATE_TIMELINE_1,
	STATE_TIMELINE_2_MISS,
	STATE_TIMELINE_2_HIT,
	STATE_TIMELINE_3,
	STATE_TIMELINE_4,
	STATE_TIMELINE_5,
	STATE_TIMELINE_6,
	STATE_ASK_FOR_CROSS,
	STATE_ATTACKED,
	STATE_AFTER_ATTACK,
	STATE_TURN_END,
	STATE_SHOW_HAND,
	STATE_HAND_CHANGE,
	STATE_DISCARD_HAND,
	STATE_TRUE_LOSE_MORALE,
	STATE_GAME_OVER
};

enum ROLE_STRATEGY{
	ROLE_STRATEGY_RANDOM,
	ROLE_STRATEGY_31,
	ROLE_STRATEGY_BP
};
//Contexts

typedef struct{
	int actionFlag;
	bool canGiveUp;
}CONTEXT_ACTION_PHASE;

typedef struct{
	int cardID;
	int dstID;
	int srcID;
	bool isActive;
}CONTEXT_TIMELINE_2_MISS;

typedef struct{
	int cardID;
	int dstID;
	int srcID;
	bool isActive;
}CONTEXT_ATTACK_ACTION;

typedef struct{
	CONTEXT_ATTACK_ACTION attack;
	HARM harm;
	int hitRate;
}CONTEXT_TIMELINE_1;

typedef struct{
	CONTEXT_ATTACK_ACTION attack;
	HARM harm;
}CONTEXT_TIMELINE_2_HIT;

typedef struct{
	HARM harm;
	int dstID;
}CONTEXT_TIMELINE_3;

typedef struct{
	HARM harm;
	int dstID;
	int crossAvailable;
}CONTEXT_TIMELINE_4;

typedef struct{
	HARM harm;
	int dstID;
}CONTEXT_TIMELINE_5;

typedef struct{
	HARM harm;
	int dstID;
}CONTEXT_TIMELINE_6;

typedef struct{
	int howMany;
	HARM harm;
	int dstID;
}CONTEXT_LOSE_MORALE;

//Reply
typedef struct{
	bool yes;
}REPLY_YESNO;

typedef struct{
	int actionFlag;
	int cardID;
	int dstID;
	int srcID;
}REPLY_ATTACK;

typedef struct{
	int flag;
	int cardID;
	int dstID;
	int srcID;
}REPLY_ATTACKED;

class GrailState
{
public:
	int state;
	int step;
	int iterator;
	GrailState(int s): state(s), step(0), iterator(0) {}
	virtual int handle(GameGrail* engine) { return GE_EMPTY_HANDLE; }
};

class StateWaitForEnter : public GrailState
{
public:
	StateWaitForEnter(): GrailState(STATE_WAIT_FOR_ENTER){}
	int handle(GameGrail* engine);
};

class StateSeatArrange : public GrailState
{
public:
	StateSeatArrange(): GrailState(STATE_SEAT_ARRANGE), isSet(false){}
	int handle(GameGrail* engine);
private:
	string msgs[MAXPLAYER];
	bool isSet;
};

class StateRoleStrategyRandom : public GrailState
{
public:
	StateRoleStrategyRandom(): GrailState(STATE_ROLE_STRATEGY_RANDOM){}
	int handle(GameGrail* engine);
};

class StateGameStart : public GrailState
{
public:
	StateGameStart(): GrailState(STATE_GAME_START), isSet(false){}
	int handle(GameGrail* engine);
	bool isSet;
};

class StateBeforeTurnBegin : public GrailState
{
public:
	StateBeforeTurnBegin(): GrailState(STATE_BEFORE_TURN_BEGIN){}
	int handle(GameGrail* engine);
};

class StateTurnBegin : public GrailState
{
public:
	StateTurnBegin(): GrailState(STATE_TURN_BEGIN){}
	int handle(GameGrail* engine);
};

class StateWeaken : public GrailState
{
public:
	StateWeaken(int howMany): GrailState(STATE_WEAKEN), howMany(howMany){}
	int handle(GameGrail* engine);
	int howMany;
};

class StateBeforeAction : public GrailState
{
public:
	StateBeforeAction(bool to = true): GrailState(STATE_BEFORE_ACTION), toAction(to){}
	int handle(GameGrail* engine);
	bool toAction;
};

class StateBoot : public GrailState
{
public:
	StateBoot(): GrailState(STATE_BOOT){}
	int handle(GameGrail* engine);
};
 
class StateActionPhase: public GrailState
{
public:
	StateActionPhase(): GrailState(STATE_ACTION_PHASE), isSet(false){}
	int handle(GameGrail* engine);
	int actionFlag;
	bool canGiveUp;
	bool isSet;
};

class StateBeforeAttack: public GrailState
{
public:
	StateBeforeAttack(int cardID, int dstID, int srcID): GrailState(STATE_BEFORE_ATTACK), cardID(cardID), dstID(dstID), srcID(srcID){}
	int handle(GameGrail* engine);
	int cardID;
	int dstID;
	int srcID;
};

class StateAfterAttack: public GrailState
{
public:
	StateAfterAttack(int cardID, int dstID, int srcID): GrailState(STATE_AFTER_ATTACK), cardID(cardID), dstID(dstID), srcID(srcID){}
	int handle(GameGrail* engine);
	int cardID;
	int dstID;
	int srcID;
};

class StateTimeline1 : public GrailState
{
public:
	StateTimeline1(CONTEXT_TIMELINE_1 *con): GrailState(STATE_TIMELINE_1), context(con){}
	~StateTimeline1(){ SAFE_DELETE(context); }
	int handle(GameGrail* engine);
	CONTEXT_TIMELINE_1 *context;
};

class StateAttacked : public GrailState
{
public:
	StateAttacked(CONTEXT_TIMELINE_1 *con): GrailState(STATE_ATTACKED), context(con){}
	~StateAttacked(){ SAFE_DELETE(context); }
	int handle(GameGrail* engine);
	CONTEXT_TIMELINE_1 *context;
};

class StateTurnEnd: public GrailState
{
public:
	StateTurnEnd(): GrailState(STATE_TURN_END){}
	int handle(GameGrail* engine);
};

class StateTimeline2Hit : public GrailState
{
public:
	StateTimeline2Hit(CONTEXT_TIMELINE_2_HIT *con): GrailState(STATE_TIMELINE_2_HIT), context(con){}
	~StateTimeline2Hit(){ SAFE_DELETE(context); }
	int handle(GameGrail* engine);
	CONTEXT_TIMELINE_2_HIT *context;
};

class StateTimeline2Miss : public GrailState
{
public:
	StateTimeline2Miss(CONTEXT_TIMELINE_2_MISS *con): GrailState(STATE_TIMELINE_2_MISS), context(con){}
	~StateTimeline2Miss(){ SAFE_DELETE(context); }
	int handle(GameGrail* engine);
	CONTEXT_TIMELINE_2_MISS *context;
};

class StateTimeline3 : public GrailState
{
public:
	StateTimeline3(CONTEXT_TIMELINE_3 *con): GrailState(STATE_TIMELINE_3), context(con){}
	~StateTimeline3(){ SAFE_DELETE(context); }
	int handle(GameGrail* engine);
	CONTEXT_TIMELINE_3 *context;
};

class StateTimeline4 : public GrailState
{
public:
	StateTimeline4(CONTEXT_TIMELINE_4 *con): GrailState(STATE_TIMELINE_4), context(con){}
	~StateTimeline4(){ SAFE_DELETE(context); }
	int handle(GameGrail* engine);
	CONTEXT_TIMELINE_4 *context;
};

class StateTimeline5 : public GrailState
{
public:
	StateTimeline5(CONTEXT_TIMELINE_5 *con): GrailState(STATE_TIMELINE_5), context(con){}
	~StateTimeline5(){ SAFE_DELETE(context); }
	int handle(GameGrail* engine);
	CONTEXT_TIMELINE_5 *context;
};

class StateTimeline6 : public GrailState
{
public:
	StateTimeline6(CONTEXT_TIMELINE_6 *con): GrailState(STATE_TIMELINE_6), context(con){}
	~StateTimeline6(){ SAFE_DELETE(context); }
	int handle(GameGrail* engine);
	CONTEXT_TIMELINE_6 *context;
};

class StateAskForCross: public GrailState
{
public:
	StateAskForCross(int dstID, HARM harm, int cross): GrailState(STATE_ASK_FOR_CROSS), dstID(dstID), harm(harm), crossAvailable(cross){}
	int handle(GameGrail* engine);
	int dstID;
	HARM harm;
	int crossAvailable;
};

class StateHandChange : public GrailState
{
public:
	StateHandChange(int dstID): GrailState(STATE_HAND_CHANGE), dstID(dstID){}
	int handle(GameGrail* engine);
	int dstID;
};

class StateDiscardHand : public GrailState
{
public:
	StateDiscardHand(int dstID, int howMany, HARM harm, bool toDemoralize, bool isShown = false): GrailState(STATE_DISCARD_HAND),
	dstID(dstID), howMany(howMany), harm(harm), toDemoralize(toDemoralize), isShown(isShown){}
	int handle(GameGrail* engine);
	int dstID;
	int howMany;
	HARM harm;
	bool toDemoralize;
	bool isShown;
};

class StateTrueLoseMorale: public GrailState
{
public:
	StateTrueLoseMorale(CONTEXT_LOSE_MORALE *con): GrailState(STATE_TRUE_LOSE_MORALE), context(con){}
	~StateTrueLoseMorale(){ SAFE_DELETE(context); }
	int handle(GameGrail* engine);
	CONTEXT_LOSE_MORALE *context;
};

class StateShowHand: public GrailState
{
public:
	StateShowHand(int dstID, int howMany, vector<int> cards): GrailState(STATE_SHOW_HAND), dstID(dstID), howMany(howMany), cards(cards){}
	int handle(GameGrail* engine);
	int dstID;
	int howMany;
	vector<int> cards;
};

class StateGameOver: public GrailState
{
public:
	StateGameOver(int winner): GrailState(STATE_GAME_OVER), color(winner){}
	int handle(GameGrail* engine);
	int color;
};
