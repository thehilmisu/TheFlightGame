#ifndef TIMERS_H
#define TIMERS_H

#include <unordered_map>
#include <string>

struct Timer {
	float time = 0.0f;
	float maxtime = 0.0f;
	//The value 'time' started at. Spawn timers are deliberately registered with
	//a start time of 0 so that the first spawn happens straight away rather
	//than after a full interval, so a full reset has to restore this and not
	//'maxtime'.
	float starttime = 0.0f;
};

class TimerManager {
  public:

    static TimerManager& getInstance() {
      static TimerManager instance;
      return instance;
    }

    TimerManager(const TimerManager&) = delete;
    TimerManager& operator=(const TimerManager&) = delete;
    
		void addTimer(const std::string& name, float maxtime);
		void addTimer(const std::string& name, float time, float maxtime);
		void update(float dt);
		//Resets all timers that have time below 0.0
		void reset();
		//Resets every timer back to the value it was registered with, used when
		//a run restarts so the next run behaves exactly like a fresh one
		void resetAll();
		bool getTimer(const std::string &name);

  private:
    TimerManager() {}
    ~TimerManager() {}
    std::unordered_map<std::string, Timer> timers;
  
};

#endif
