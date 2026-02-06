#ifndef TIMERS_H
#define TIMERS_H

#include <unordered_map>
#include <string>

struct Timer {
	float time = 0.0f;
	float maxtime = 0.0f;
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
		bool getTimer(const std::string &name);

  private:
    TimerManager() {}
    ~TimerManager() {}
    std::unordered_map<std::string, Timer> timers;
  
};

#endif
