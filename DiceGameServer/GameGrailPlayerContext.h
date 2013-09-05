#pragma once

#include "PlayerContext.h"

class GameGrailPlayerContext : public PlayerContext
{
public:
	GameGrailPlayerContext(string userId) : PlayerContext(userId), buf(NULL)
	{}
	~GameGrailPlayerContext() {
		if(buf){
			delete buf;
		}
	}
	void setBuf(void *reply){
		if(buf){
			delete buf;
		}
		buf = reply;
	}
	void* getBuf(){
		return buf;
	}
private:
	void* buf;
};
