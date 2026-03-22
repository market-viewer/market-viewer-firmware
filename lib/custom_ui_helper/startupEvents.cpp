#include "startupEvents.h"
#include "ui.h"
#include <Arduino.h>
#include "WifiConfig.h"
#include "HardwareDriver.h"
#include "AudioManager.h"
#include <Preferences.h>
#include "ScreensManager.h"
#include "ui_events_helper.h"

static Preferences preferences;

// Default values
#define DEFAULT_BRIGHTNESS 50
#define DEFAULT_VOLUME 50
#define DEFAULT_ROTATION true

void loadHardwareSettingsFromPreferences() {
    preferences.begin("hardware_config", true);  // Read-only
    
    // Load brightness
    int savedBrightness = preferences.getInt("brightness", DEFAULT_BRIGHTNESS);
    loadBrightnessFromPreferences(savedBrightness);
    
    // Load volume
    int savedVolume = preferences.getInt("volume", DEFAULT_VOLUME);
    loadVolumeFromPreferences(savedVolume);
    
    // Load rotation setting
    loadRotationFromPreferences(preferences.getBool("rotation", DEFAULT_ROTATION));    
    preferences.end();
    
    Serial.println("Hardware settings loaded from preferences");
}

void loadBrightnessFromPreferences(int value) {
    currentBrightness = value;
    set_brightness_percentage(value);
    //update ui
    lv_arc_set_value(ui_brightnessArc, value);
    lv_label_set_text_fmt(ui_brightnessValueLabel, "%d%%", value);
    Serial.println("Loaded brightness: " + String(value));
}
void loadVolumeFromPreferences(int value) {
    currentVolume = value;
    set_volume_percentage(value);

    //update ui
    lv_arc_set_value(ui_volumeArc, value);
    lv_label_set_text_fmt(ui_volumeValueLabel, "%d%%", value);
    Serial.println("Loaded volume: " + String(value));
}
void loadRotationFromPreferences(bool value) {
    shouldRotate = value;
    if (shouldRotate) {
        lv_obj_add_state(ui_rotateButton, LV_STATE_CHECKED);
    } else {
        lv_obj_clear_state(ui_rotateButton, LV_STATE_CHECKED);
    }
    
    Serial.println("Loaded rotation: " + String(shouldRotate ? "Yes" : "No"));

}

void loadScreensOnStartup() {
    if (!is_wifi_connected()) return;

    bool successful = get_screens_from_backend();

    updateScreensScreenOnDataFetch(successful);
}