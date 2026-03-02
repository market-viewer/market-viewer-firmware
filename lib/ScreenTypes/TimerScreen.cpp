#include "TimerScreen.h"
#include "ui.h"
#include "AudioManager.h"

#define ICON_PLAY &ui_img_1970864515
#define ICON_PAUSE &ui_img_81954201

const char* endScreenDisplayText = "End!";

void TimerScreen::render() {
    //update the vlaues
    lv_label_set_text(ui_timerNameLabel, name.c_str());

    if (isRunning) {
        startTimerUIUpdate();
        //update the arc and times
        if (isStopWatch) {
            updateStopwatchUI();
        } else {
            updateTimerUI();
        }
        //update the play button
        if (isPaused) {
            lv_obj_set_style_bg_img_src(ui_timerPlayPauseButton, ICON_PLAY, LV_PART_MAIN);
        }

        //handle timer end screen
        if (isTimerAtZero()) {
            lv_label_set_text(ui_timerTimeLabel, endScreenDisplayText);
            lv_obj_add_state(ui_timerPlayPauseButton, LV_STATE_DISABLED);
        } else {
            lv_obj_clear_state(ui_timerPlayPauseButton, LV_STATE_DISABLED);
        }
        
    } else {
        resetTimerUIUpdate();
    }
}

void TimerScreen::resetTimer() {
    // if the timer is running go to timer setup secreen if not reset the values to 0
    if (isRunning) {
        resetRunningTimer();
    } else {
        lv_roller_set_selected(ui_hourRoller, 0, LV_ANIM_ON);
        lv_roller_set_selected(ui_minuteRoller, 0, LV_ANIM_ON);
        lv_roller_set_selected(ui_secondRoller, 0, LV_ANIM_ON);
    }

}

void TimerScreen::loadTimerValues() {
    hour = lv_roller_get_selected(ui_hourRoller);
    minute = lv_roller_get_selected(ui_minuteRoller);
    second = lv_roller_get_selected(ui_secondRoller);
    timerTotalSeconds = hour * 60 * 60 + minute * 60 + second; 
}

void TimerScreen::startTimerUIUpdate() {
    lv_obj_add_flag(ui_hourRoller, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui_minuteRoller, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui_secondRoller, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui_timerDescriptionSelectorLabel, LV_OBJ_FLAG_HIDDEN);
    
    lv_obj_clear_flag(ui_timerArc, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(ui_timerTimeLabel, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(ui_timerIcon, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(ui_timerNameLabel, LV_OBJ_FLAG_HIDDEN);

    char timeText[16];
    snprintf(timeText, sizeof(timeText), "%02d:%02d:%02d", hour, minute, second);
    lv_label_set_text(ui_timerTimeLabel, timeText);

    //update the play button icon
    lv_obj_set_style_bg_img_src(ui_timerPlayPauseButton, ICON_PAUSE, LV_PART_MAIN);
}

void TimerScreen::resetRunningTimer() {
    isRunning = false;
    isPaused = false;
    resetTimerUIUpdate();
}

void TimerScreen::resetTimerUIUpdate() {
    lv_obj_clear_flag(ui_hourRoller, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(ui_minuteRoller, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(ui_secondRoller, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(ui_timerDescriptionSelectorLabel, LV_OBJ_FLAG_HIDDEN);

    lv_obj_add_flag(ui_timerArc, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui_timerTimeLabel, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui_timerIcon, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui_timerNameLabel, LV_OBJ_FLAG_HIDDEN);

    //update the play button icon
    lv_obj_set_style_bg_img_src(ui_timerPlayPauseButton, ICON_PLAY, LV_PART_MAIN);
    lv_obj_clear_state(ui_timerPlayPauseButton, LV_STATE_DISABLED);
}

void TimerScreen::timerEndUIUpdate() {
    lv_label_set_text(ui_timerTimeLabel, endScreenDisplayText);
    lv_obj_add_state(ui_timerPlayPauseButton, LV_STATE_DISABLED);
}


void TimerScreen::startTimer() {
    loadTimerValues();
    startTimerUIUpdate();

    isRunning = true;
    isStopWatch = isTimerAtZero();

    last_tick = millis() - 1000;
}

bool TimerScreen::updateTimerScreen(bool updateUI) {
    if (isStopWatch) {
        updateStopwatch(updateUI);
        return false;
    } else {
        return updateTimer(updateUI);
    }
}

bool TimerScreen::updateTimer(bool updateUI) {
    if (!isRunning || isPaused) {return false;}

    uint32_t currentMillis = millis();

    if (currentMillis - last_tick >= 1000) {
        int seconds_passed = (currentMillis - last_tick) / 1000;
        last_tick += (seconds_passed * 1000);

        int current_total = (hour * 3600) + (minute * 60) + second;
        current_total -= seconds_passed;

        hour = current_total / 3600;
        minute = (current_total % 3600) / 60;
        second = current_total % 60;
       
        if (current_total <= 0) {
            hour = 0; minute = 0; second = 0;
            if (updateUI) updateTimerUI();
            
            timerEndUIUpdate();
            play_beep(1500, 400);
            isPaused = true;
            return true;
        } else {
            hour = current_total / 3600;
            minute = (current_total % 3600) / 60;
            second = current_total % 60;
        }

        if (updateUI) updateTimerUI();
    }

    return false;
}

void TimerScreen::updateTimerUI() {
    char timeText[16];

    if(minute == 0 && hour == 0) {
        snprintf(timeText, sizeof(timeText), "%02d", second);
    } else if (hour == 0) {
        snprintf(timeText, sizeof(timeText), "%02d:%02d", minute, second);
    } else {
        snprintf(timeText, sizeof(timeText), "%02d:%02d:%02d", hour, minute, second);
    }
    

    //update digial clock
    lv_label_set_text(ui_timerTimeLabel, timeText);

    //update arc
    int remainingSeconds = hour * 60 * 60 + minute * 60 + second;
    int arcValue =  (remainingSeconds * 1000) / timerTotalSeconds;

    lv_arc_set_value(ui_timerArc, arcValue);
}

void TimerScreen::updateStopwatch(bool updateUI) {
    if (!isRunning || isPaused) return;

    uint32_t currentMillis = millis();
    
    if (currentMillis - last_tick >= 1000) {
        int seconds_passed = (currentMillis - last_tick) / 1000;
        last_tick += (seconds_passed * 1000);

        int current_total = (hour * 3600) + (minute * 60) + second;
        current_total += seconds_passed; // Add time for stopwatch

        hour = current_total / 3600;
        minute = (current_total % 3600) / 60;
        second = current_total % 60;

        if (updateUI) updateStopwatchUI();
    }
}

void TimerScreen::updateStopwatchUI() {
    char timeText[16];

    if(minute == 0 && hour == 0) {
        snprintf(timeText, sizeof(timeText), "%02d", second);
    } else if (hour == 0) {
        snprintf(timeText, sizeof(timeText), "%02d:%02d", minute, second);
    } else {
        snprintf(timeText, sizeof(timeText), "%02d:%02d:%02d", hour, minute, second);
    }
    
    //update digial clock
    lv_label_set_text(ui_timerTimeLabel, timeText);
    int arcValue =  second * 1000 / 60;
    lv_arc_set_value(ui_timerArc, arcValue);
}

void TimerScreen::togglePauseTimer() {
    if (isPaused) {
        isPaused = false;
        last_tick = millis();
        lv_obj_set_style_bg_img_src(ui_timerPlayPauseButton, ICON_PAUSE, LV_PART_MAIN);
    } else {
        isPaused = true;
        lv_obj_set_style_bg_img_src(ui_timerPlayPauseButton, ICON_PLAY, LV_PART_MAIN);
    }

}

bool TimerScreen::isTimerAtZero() {
    return hour == 0 && minute == 0 && second == 0;
}
