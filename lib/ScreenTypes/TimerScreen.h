#ifndef TIMER_SCREEN_H
#define TIMER_SCREEN_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "BaseScreen.h"

class TimerScreen : public BaseScreen {
private:
    String name;
    bool isRunning;
    bool isStopWatch;
    bool isPaused;

    int hour, minute, second;
    int timerTotalSeconds;

    uint32_t last_tick;


public:
    TimerScreen(int pos, String name) : BaseScreen(pos), name(name) {
        type = ScreenType::TIMER;
        isRunning = false;
        isStopWatch = false;
        isPaused = false;
    }

    void render() override;
    void parseData(JsonObject& data) override {}
    bool needsUpdate() override {return false;}

    //updates timer screen and returns true if timer ends otherwise false
    bool updateTimerScreen(bool updateUI);
    
    bool updateTimer(bool updateUI);
    void updateTimerUI();

    void updateStopwatch(bool updateUI);
    void updateStopwatchUI();

    void resetTimer();
    void startTimer();
    void togglePauseTimer(); 

    void loadTimerValues();
    void startTimerUIUpdate();
    void resetTimerUIUpdate();
    void resetRunningTimer();

    void timerEndUIUpdate();

    bool isTimerAtZero();

    bool isTimerRunning() {return isRunning;}
    bool isTimerPaused() {return isPaused;}

    String getDisplayName() override {
        return "Timer (" + name + ")";
    }
};



#endif