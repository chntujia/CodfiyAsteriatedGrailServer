#pragma once

#include "PlayerContext.h"

class GameGrailPlayerContext : public PlayerContext
{
public:
	GameGrailPlayerContext(std::string userId) : PlayerContext(userId), ready(false), buf(NULL)
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
	void setReady(bool r) { ready = r; }
	bool isReady() { return ready; }
	void setName(std::string n) { name = n; }
	std::string getName() { return name; }
private:
	bool ready;
	std::string name;
	void* buf;
};
