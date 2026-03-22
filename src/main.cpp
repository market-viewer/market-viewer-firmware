#include <Arduino.h>
#include "HardwareDriver.h"
#include "AudioManager.h"
#include "GuiManager.h"
#include "WifiConfig.h"
#include "startupEvents.h"
#include "ScreensManager.h"
#include "ui_events.h"

void setup() {
  Serial.begin(115200);
  
  //initialize Hardware (Screen, Touch, I2C)
  init_hardware();

  //initialize audio 
  init_audio_manager();
  
  //try to connect to saved wifi
  init_wifi_config();
  
  //initialize LVGL (Buffers, UI)
  init_lvgl_interface();

  // load saved hardware settings (brightness, volume, rotation)
  loadHardwareSettingsFromPreferences();

  //load screens on startup
  loadScreensOnStartup();
  
  Serial.println("System Ready!");
}

void loop() {
    update_gui();
  
    updateScreen();

    delay(5);
}