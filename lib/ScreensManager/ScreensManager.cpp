#include "ScreensManager.h"
#include "HttpRequestManager.h"
#include "WifiConfig.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include "ClockScreen.h"
#include "TimerScreen.h"

static std::vector<BaseScreen*> screens;
static Preferences preferences;

void init_screens_manager() {
    clear_all_screens();

    //default screens (clock and timer), when none are fetched
    add_default_screens();
}

void add_default_screens() {
    screens.push_back(new ClockScreen(0, "Europe/London", "GMT0BST,M3.5.0/1,M10.5.0", true, ClockType::ANALOG_CLOCK));
    screens.push_back(new TimerScreen(1, "Timer 1"));
}

void clear_all_screens() {
    for (auto screen : screens) {
        delete screen;
    }
    screens.clear();
    
    Serial.println("All screens cleared");
}

void sortScreens() {
    std::sort(screens.begin(), screens.end(), [](const BaseScreen* lhs, const BaseScreen* rhs) {
        return lhs->getPosition() < rhs->getPosition();
    });
}


bool get_screens_from_backend() {
    JsonDocument doc;

    if (!fetch_screens(doc)) {
        return false;
    }
    JsonArray screensArray = doc.as<JsonArray>();
    
    if (screensArray.size() == 0) {
        Serial.println("No screens found in response");
        return false;
    }
    
    clear_all_screens();
    
    for (JsonObject screenObj : screensArray) {
        String type = screenObj["screenType"];
        JsonObject data = screenObj;
                
        // create screen using factory
        BaseScreen* screen = createScreenFromType(type, data);
        
        if (screen != nullptr) {
            screens.push_back(screen);
            Serial.println("Added screen: " + screen->getDisplayName());
        } else {
            Serial.println("Failed to create screen of type: " + type);
        }
    }

    

    //sort screens
    sortScreens();
    
    Serial.println("Fetched " + String(screens.size()) + " screens");    
    return true;
}

std::vector<ScreenInfo> get_all_screens_info() {
    std::vector<ScreenInfo> allScreenInfo = {};
    for (auto screen : screens) {
        allScreenInfo.push_back(ScreenInfo(screen->getPosition(), screen->getDisplayName()));
    }

    return allScreenInfo;
}

int get_screen_count() {
    return screens.size();
}

BaseScreen* get_screen_ptr(int index) {
    if(index >= 0 && index < screens.size()) {
        return screens[index];
    }
    return nullptr;
}

void updateActiveScreen(int activeScreenIndex) {
    if (activeScreenIndex == -1) return; // home screen

    BaseScreen* activeScreen = get_screen_ptr(activeScreenIndex);
    if (!activeScreen) return;

    if (activeScreen->needsUpdate()) {
        activeScreen->update();
    }

    // updale clock
    if (activeScreen->getType() == ScreenType::CLOCK) {
        ClockScreen* clock = static_cast<ClockScreen*>(activeScreen);
        clock->updateClockTimeDisplay();
    }

    // update active timer
    if (activeScreen->getType() == ScreenType::TIMER) {
        TimerScreen* timerScreen = static_cast<TimerScreen*>(activeScreen);
        timerScreen->updateTimerScreen(true);
    }
}

int updateAllTimers() {
    int timerEndIndex = -1;
    for (int i = 0; i < screens.size(); ++i) {
        BaseScreen* screen = screens[i];
        if (!screen) return -1; //if we fetched new screens 

        if (screen->getType() == ScreenType::TIMER) {
            TimerScreen* timer = static_cast<TimerScreen*>(screen);
            //update the timer screen and check if the timer ends -> go the the timer screen
            if(timer->updateTimerScreen(false)) {
                timerEndIndex = i;
            }
        }
    }

    return timerEndIndex;
}