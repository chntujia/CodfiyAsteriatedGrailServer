#pragma once
#include "GameGrailCommon.h"
// game state define

enum STATE{
	STATE_FATAL_ERROR,
	STATE_WAIT_FOR_ENTER,
	STATE_SEAT_ARRANGE,	
	STATE_ROLE_STRATEGY_RANDOM,
	STATE_ROLE_STRATEGY_31,
	STATE_ROLE_STRATEGY_ANY,
	STATE_ROLE_STRATEGY_BP,
	STATE_ROLE_STRATEGY_CM,
	STATE_GAME_START,
	STATE_BEFORE_TURN_BEGIN,
	STATE_TURN_BEGIN,
	STATE_TURN_BEGIN_SHIREN,
	STATE_WEAKEN,
	STATE_BETWEEN_WEAK_AND_ACTION,
	STATE_BEFORE_ACTION,
	STATE_BOOT,	
	STATE_ACTION_PHASE,
	STATE_BEFORE_ATTACK,
	STATE_ATTACKED,
	STATE_ATTACK_SKILL,
	STATE_AFTER_ATTACK,
	STATE_BEFORE_MAGIC,
	STATE_MISSILED,
	STATE_MAGIC_SKILL,
	STATE_AFTER_MAGIC,
	STATE_BEFORE_SPECIAL,
	STATE_SPECIAL_SKILL,
	STATE_AFTER_SPECIAL,
	STATE_ADDITIONAL_ACTION,
	STATE_TURN_END,
	STATE_TIMELINE_1,
	STATE_TIMELINE_2_MISS,
	STATE_TIMELINE_2_HIT,
	STATE_HARM_END,
	STATE_TIMELINE_3,
	STATE_TIMELINE_4,
	STATE_TIMELINE_5,
	STATE_TIMELINE_6,
	STATE_TIMELINE_6_DRAWN,
	STATE_ASK_FOR_CROSS,
	STATE_SHOW_HAND,
	STATE_HAND_CHANGE,
	STATE_BASIC_EFFECT_CHANGE,
	STATE_COVER_CHANGE,
	STATE_REQUEST_HAND,
	STATE_REQUEST_COVER,
	STATE_BEFORE_LOSE_MORALE,
	STATE_LOSE_MORALE,
	STATE_FIX_MORALE,
	STATE_TRUE_LOSE_MORALE,
	STATE_GAME_OVER
};

//Contexts

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
    bool checkShield;
}CONTEXT_TIMELINE_1;

typedef struct{
	CONTEXT_ATTACK_ACTION attack;
	HARM harm;
}CONTEXT_TIMELINE_2_HIT;

typedef struct{
	HARM harm;
	int dstID;
}CONTEXT_HARM_END;

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
	HARM harm;
	int dstID;
}CONTEXT_TIMELINE_6_DRAWN;

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

class GameGrail;

class GrailState
{
public:
	const int state;
	int step;
	int iterator;
	GrailState(int s): state(s), step(STEP_INIT), iterator(0), errorCount(0) {}
	virtual ~GrailState() {}
	void moveIterator(int ret) {
		if(GE_SUCCESS == ret || STEP_DONE == step){
			iterator++;
			step = STEP_INIT;
		}
	}
	virtual int handle(GameGrail* engine) = 0;	
	int getErrorCount() { return errorCount; }
	void increaseErrorCount() { errorCount++; }
private:
	int errorCount;
};

class StateWaitForEnter : public GrailState
{
public:
	StateWaitForEnter(): GrailState(STATE_WAIT_FOR_ENTER), isSet(false){}
	int handle(GameGrail* engine);
private:
	bool isSet;
};

class StateSeatArrange : public GrailState
{
public:
	StateSeatArrange(): GrailState(STATE_SEAT_ARRANGE), isSet(false){}
	int handle(GameGrail* engine);
private:
	void assignTeam(GameGrail* engine);
	vector< int > assignColor(int mode, int playerNum);

	bool isSet;
	vector< int > red;
	vector< int > blue;
	int red_leader;
	int blue_leader;
	GameInfo* messages[MAXPLAYER];
};

class StateRoleStrategyRandom : public GrailState
{
public:
	StateRoleStrategyRandom(): GrailState(STATE_ROLE_STRATEGY_RANDOM){}
	int handle(GameGrail* engine);
};

class StateRoleStrategy31 : public GrailState
{
public:
	StateRoleStrategy31(): GrailState(STATE_ROLE_STRATEGY_31), isSet(false){ memset(messages, 0, sizeof(messages)); }
	~StateRoleStrategy31() { for(int i = 0; i < MAXPLAYER; i++) SAFE_DELETE(messages[i]);}
	int handle(GameGrail* engine);
private:
	bool isSet;
	RoleRequest* messages[MAXPLAYER];
};

class StateRoleStrategyAny : public GrailState
{
public:
	StateRoleStrategyAny(): GrailState(STATE_ROLE_STRATEGY_ANY), isSet(false){ memset(messages, 0, sizeof(messages)); }
	~StateRoleStrategyAny() { for(int i = 0; i < MAXPLAYER; i++) SAFE_DELETE(messages[i]);}
	int handle(GameGrail* engine);
private:
	bool isSet;
	RoleRequest* messages[MAXPLAYER];
};

class StateRoleStrategyBP: public GrailState
{
private:
	int step;
	int* alternativeRoles;
	int* options;
	vector<int> red;
	vector<int> blue;
	int alternativeNum;
	int playerNum;
public:
	StateRoleStrategyBP(): GrailState(STATE_ROLE_STRATEGY_BP){ alternativeRoles = NULL; options = NULL; step = 0;}
	~StateRoleStrategyBP() { SAFE_DELETE(alternativeRoles);SAFE_DELETE(options);}
	int handle(GameGrail* engine);
};

class StateRoleStrategyCM: public GrailState
{
private:
	int step;
	int* alternativeRoles;
	int* options;
	vector<int> red;
	vector<int> blue;
	int idr;
	int idb;
	int ibb;
	int ibr;
	bool decided;		
	int alternativeNum;
	int playerNum;
public:
	StateRoleStrategyCM(): GrailState(STATE_ROLE_STRATEGY_CM){ alternativeRoles = NULL; options = NULL; step = 0;}
	~StateRoleStrategyCM() { SAFE_DELETE(alternativeRoles);SAFE_DELETE(options);}
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

class StateTurnBeginShiRen : public GrailState
{
public:
	StateTurnBeginShiRen(): GrailState(STATE_TURN_BEGIN_SHIREN){}
	int handle(GameGrail* engine);
};

class StateWeaken : public GrailState
{
public:
	StateWeaken(int srcID, int howMany): GrailState(STATE_WEAKEN), srcID(srcID), howMany(howMany){}
	int handle(GameGrail* engine);
	int srcID;
	int howMany;	
};

class StateBetweenWeakAndAction : public GrailState
{
public:
	StateBetweenWeakAndAction(): GrailState(STATE_BETWEEN_WEAK_AND_ACTION){}
	int handle(GameGrail* engine);
};

class StateBeforeAction : public GrailState
{
public:
	StateBeforeAction(): GrailState(STATE_BEFORE_ACTION){}
	int handle(GameGrail* engine);
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
	StateActionPhase(int allowAction, bool canGiveUp): GrailState(STATE_ACTION_PHASE), allowAction(allowAction), canGiveUp(canGiveUp){}
	int handle(GameGrail* engine);
	int allowAction;
	bool canGiveUp;
private:
	int basicAttack(Action *action, GameGrail* engine);
	int basicMagic(Action *action, GameGrail* engine);
	int basicSpecial(Action *action, GameGrail* engine);
	int magicSkill(Action *action, GameGrail* engine);
	int attackSkill(Action *action, GameGrail* engine);
	int specialSkill(Action *action, GameGrail* engine);
	int unactional(Action *action, GameGrail* engine);
};

class StateBeforeAttack: public GrailState
{
public:
	StateBeforeAttack(int dstID, int srcID): GrailState(STATE_BEFORE_ATTACK), dstID(dstID), srcID(srcID){}
	int handle(GameGrail* engine);
	int dstID;
	int srcID;
};

class StateAttacked : public GrailState
{
public:
	StateAttacked(CONTEXT_TIMELINE_1 *con): GrailState(STATE_ATTACKED), context(con){}
	~StateAttacked(){ SAFE_DELETE(context); }
	int handle(GameGrail* engine);
	CONTEXT_TIMELINE_1 *context;
};

class StateAttackSkill: public GrailState
{
public:
	StateAttackSkill(Action *_action): GrailState(STATE_ATTACK_SKILL)
	{
		action = new Action;
		action->CopyFrom(*_action);
	}
	int handle(GameGrail* engine);
	~StateAttackSkill()
	{
		delete action;
	}
private:
	Action *action;
};

class StateAfterAttack: public GrailState
{
public:
	StateAfterAttack(int srcID): GrailState(STATE_AFTER_ATTACK), srcID(srcID){}
	int handle(GameGrail* engine);
	int srcID;
};

class StateBeforeMagic: public GrailState
{
public:
	StateBeforeMagic(int srcID): GrailState(STATE_BEFORE_MAGIC), srcID(srcID){}
	int handle(GameGrail* engine);
	int srcID;
};

class StateMissiled: public GrailState
{
public:
	StateMissiled(int dstID, int srcID, bool isClockwise): GrailState(STATE_MISSILED), dstID(dstID), srcID(srcID), isClockwise(isClockwise), harmPoint(2){
	    memset(hasMissiled, 0, sizeof(hasMissiled));
		hasMissiled[srcID] = true;
	}
	static StateMissiled* create(GameGrail* engine, int cardID, int dstID, int srcID);	
	int handle(GameGrail* engine);
	int getNextTargetID(GameGrail* engine, int startID);	
private:
	int dstID;
	int srcID;
	bool isClockwise;
	int harmPoint;
	bool hasMissiled[MAXPLAYER];
};

class StateMagicSkill: public GrailState
{
public:
	StateMagicSkill(Action *_action): GrailState(STATE_MAGIC_SKILL)
	{
		action = new Action;
		action->CopyFrom(*_action);
	}
	int handle(GameGrail* engine);
	~StateMagicSkill()
	{
		delete action;
	}
private:
	Action *action;
};

class StateAfterMagic: public GrailState
{
public:
	StateAfterMagic(int srcID): GrailState(STATE_AFTER_MAGIC), srcID(srcID){}
	int handle(GameGrail* engine);
	int srcID;
};

class StateBeforeSpecial: public GrailState
{
public:
	StateBeforeSpecial(int srcID): GrailState(STATE_BEFORE_SPECIAL), srcID(srcID){}
	int handle(GameGrail* engine);
	int srcID;
};

class StateSpecialSkill: public GrailState
{
public:
	StateSpecialSkill(Action *_action): GrailState(STATE_SPECIAL_SKILL)
	{
		action = new Action;
		action->CopyFrom(*_action);
	}
	int handle(GameGrail* engine);
	~StateSpecialSkill()
	{
		delete action;
	}
private:
	Action *action;
};

class StateAfterSpecial: public GrailState
{
public:
	StateAfterSpecial(int srcID): GrailState(STATE_AFTER_SPECIAL), srcID(srcID){}
	int handle(GameGrail* engine);
	int srcID;
};

class StateAdditionalAction: public GrailState
{
public:
	StateAdditionalAction(int srcID): GrailState(STATE_ADDITIONAL_ACTION), srcID(srcID){}
	int handle(GameGrail* engine);
	int srcID;
};

class StateTurnEnd: public GrailState
{
public:
	StateTurnEnd(): GrailState(STATE_TURN_END){}
	int handle(GameGrail* engine);
};

class StateTimeline1 : public GrailState
{
public:
	StateTimeline1(CONTEXT_TIMELINE_1 *con): GrailState(STATE_TIMELINE_1), context(con){}
	~StateTimeline1(){ SAFE_DELETE(context); }
	int handle(GameGrail* engine);
	CONTEXT_TIMELINE_1 *context;
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

class StateHarmEnd : public GrailState
{
	public:
	StateHarmEnd(CONTEXT_HARM_END *con): GrailState(STATE_HARM_END), context(con), isSet(false){}
	~StateHarmEnd(){ SAFE_DELETE(context); }
	int handle(GameGrail* engine);
	CONTEXT_HARM_END *context;
	bool isSet;
};

class StateTimeline3 : public GrailState
{
public:
	StateTimeline3(CONTEXT_TIMELINE_3 *con): GrailState(STATE_TIMELINE_3), context(con), isSet(false){}
	~StateTimeline3(){ SAFE_DELETE(context); }
	int handle(GameGrail* engine);
	CONTEXT_TIMELINE_3 *context;
	bool isSet;
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

class StateTimeline6Drawn : public GrailState
{
public:
	StateTimeline6Drawn(CONTEXT_TIMELINE_6_DRAWN *con): GrailState(STATE_TIMELINE_6_DRAWN), context(con){}
	~StateTimeline6Drawn(){ SAFE_DELETE(context); }
	int handle(GameGrail* engine);
	CONTEXT_TIMELINE_6_DRAWN *context;
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
	StateHandChange(int dstID, int direction, int howMany, vector<int> cards, HARM harm): GrailState(STATE_HAND_CHANGE), dstID(dstID), direction(direction),
	howMany(howMany), cards(cards), harm(harm), isSet(false){}
	int handle(GameGrail* engine);
	int dstID;
	int direction;
	int howMany;
	vector<int> cards;
	HARM harm;
	bool isSet;
};

class StateBasicEffectChange : public GrailState
{
public:
	StateBasicEffectChange(int dstID, int direction, int card, int doerID, int cause): GrailState(STATE_BASIC_EFFECT_CHANGE), dstID(dstID), direction(direction),
	card(card), doerID(doerID), cause(cause), isSet(false){}
	int handle(GameGrail* engine);
	int dstID;
	int direction;
	int card;
	int doerID;
	int cause;
	bool isSet;
};

class StateCoverChange : public GrailState
{
public:
	StateCoverChange(int dstID, int direction, int howMany, vector<int> cards, HARM harm): GrailState(STATE_COVER_CHANGE), dstID(dstID), direction(direction),
	howMany(howMany), cards(cards), harm(harm), isSet(false){}
	int handle(GameGrail* engine);
	int dstID;
	int direction;
	int howMany;
	vector<int> cards;
	HARM harm;
	bool isSet;
};

class StateRequestHand : public GrailState
{
public:
	StateRequestHand(int targetID, HARM harm, int dstOwner = -1, int dstArea = DECK_DISCARD, bool isShown = false, bool canGiveUp = false): GrailState(STATE_REQUEST_HAND),
		targetID(targetID), harm(harm), dstOwner(dstOwner), dstArea(dstArea), isShown(isShown), canGiveUp(canGiveUp){}
	int handle(GameGrail* engine);
	int targetID;
	HARM harm;
	int dstOwner;
	int dstArea;
	bool isShown;
	bool canGiveUp;
};

class StateRequestCover : public GrailState
{
public:
	StateRequestCover(int targetID, HARM harm, int dstOwner = -1, int dstArea = DECK_DISCARD, bool isShown = false, bool canGiveUp = false): GrailState(STATE_REQUEST_COVER),
		targetID(targetID), harm(harm), dstOwner(dstOwner), dstArea(dstArea), isShown(isShown), canGiveUp(canGiveUp){}
	int handle(GameGrail* engine);
	int targetID;
	HARM harm;
	int dstOwner;
	int dstArea;
	bool isShown;
	bool canGiveUp;
};

class StateBeforeLoseMorale: public GrailState
{
public:
	StateBeforeLoseMorale(CONTEXT_LOSE_MORALE *con): GrailState(STATE_BEFORE_LOSE_MORALE), context(con){}
	~StateBeforeLoseMorale(){ SAFE_DELETE(context); }
	int handle(GameGrail* engine);
	CONTEXT_LOSE_MORALE *context;
};

class StateLoseMorale: public GrailState
{
public:
	StateLoseMorale(CONTEXT_LOSE_MORALE *con): GrailState(STATE_LOSE_MORALE), context(con){}
	~StateLoseMorale(){ SAFE_DELETE(context); }
	int handle(GameGrail* engine);
	CONTEXT_LOSE_MORALE *context;
};

class StateFixMorale: public GrailState
{
public:
	StateFixMorale(CONTEXT_LOSE_MORALE *con): GrailState(STATE_FIX_MORALE), context(con){}
	~StateFixMorale(){ SAFE_DELETE(context); }
	int handle(GameGrail* engine);
	CONTEXT_LOSE_MORALE *context;
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
	StateShowHand(int dstID, int howMany, vector<int> cards, HARM harm): GrailState(STATE_SHOW_HAND), dstID(dstID), howMany(howMany), cards(cards), harm(harm){}
	int handle(GameGrail* engine);
	int dstID;
	int howMany;
	vector<int> cards;
	HARM harm;
};

class StateGameOver: public GrailState
{
public:
	StateGameOver(int winner): GrailState(STATE_GAME_OVER), color(winner){}
	int handle(GameGrail* engine);
	int getcolor(){ return color; }
private:
	int color;
};
