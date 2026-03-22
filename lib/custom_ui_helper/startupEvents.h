#ifndef STARTUP_EVENTS_H
#define STARTUP_EVENTS_H

#include <Arduino.h>

void loadHardwareSettingsFromPreferences();
void loadBrightnessFromPreferences(int value);
void loadVolumeFromPreferences(int value);
void loadRotationFromPreferences(bool value);

void loadScreensOnStartup();


#endif