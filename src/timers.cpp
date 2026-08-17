#include "timers.h"


void TimerManager::addTimer(const std::string &name, float maxtime){
	Timer t = {
		.time = maxtime,
		.maxtime = maxtime,
		.starttime = maxtime
	};
	timers.insert({ name, t });
}

void TimerManager::addTimer(const std::string& name, float time, float maxtime){
	Timer t = {
		.time = time,
		.maxtime = maxtime,
		.starttime = time
	};
	timers.insert({ name, t });
}

void TimerManager::update(float dt){
	for(auto &t : timers)
		t.second.time -= dt;
}

void TimerManager::reset(){
	for(auto &t : timers)
		if(t.second.time < 0.0f)
			t.second.time = t.second.maxtime;
}

void TimerManager::resetAll(){
	for(auto &t : timers)
		t.second.time = t.second.starttime;
}

bool TimerManager::getTimer(const std::string &name){
	if(!timers.count(name))
		return false;
	return timers.at(name).time < 0.0f;
}
