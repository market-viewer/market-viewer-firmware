#include "GuiManager.h"
#include "HardwareDriver.h" 
#include <pin_config.h>
#include "lv_conf.h"
#include "ui.h"
#include "ScreensManager.h"
#include "BaseScreen.h"
#include "ClockScreen.h"
#include "TimerScreen.h"


// Double Buffering for smooth UI
static lv_disp_draw_buf_t draw_buf;

// --- 1. TICK TIMER (Essential for animations) ---
void example_increase_lvgl_tick(void *arg) {
    lv_tick_inc(2); // Tell LVGL 2ms passed
}

//work but is slow
void example_lvgl_rounder_cb(struct _lv_disp_drv_t *disp_drv, lv_area_t *area)
{
    // Force the update to span the entire width of the screen
    area->x1 = 0;
    area->x2 = disp_drv->hor_res - 1;

    // Align Y to even
    if(area->y1 % 2 != 0) area->y1--;
    if(area->y2 % 2 == 0) area->y2++;
}

// void example_lvgl_rounder_cb(struct _lv_disp_drv_t *disp_drv, lv_area_t *area)
// {
//     // --- X ALIGNMENT ---
//     // 1. Round START down to nearest even number (0, 2, 4...)
//     area->x1 = area->x1 & ~1;

//     // 2. Round END up to nearest odd number (1, 3, 5...)
//     // This ensures (x2 - x1 + 1) is always an EVEN number (Width divisible by 2)
//     area->x2 = area->x2 | 1;

//     // --- Y ALIGNMENT ---
//     // Same logic for Y to keep memory pointers aligned
//     area->y1 = area->y1 & ~1;
//     area->y2 = area->y2 | 1;

//     // --- SAFETY CLIPPING (Crucial!) ---
//     // If your screen is 466px wide, the max index is 465.
//     // If x2 rounds up to 467, it will cause distortion/wrapping.
//     if (area->x2 >= disp_drv->hor_res) area->x2 = disp_drv->hor_res - 1;
//     if (area->y2 >= disp_drv->ver_res) area->y2 = disp_drv->ver_res - 1;
// }


// --- 3. FLUSH CB (With Fix for Broken Pixels) ---
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    // This block automatically handles the pixel swapping based on your Config file
    #if (LV_COLOR_16_SWAP != 0)
        gfx->draw16bitBeRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
    #else
        gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
    #endif

    lv_disp_flush_ready(disp);
}

// --- 4. INPUT CB ---
void my_touchpad_read(lv_indev_drv_t * drv, lv_indev_data_t * data) {
    int16_t x, y;
    if (get_touch(x, y)) {
        data->state = LV_INDEV_STATE_PR;
        data->point.x = x;
        data->point.y = y;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
}


void handle_screen_rotation() {
    if (!shouldRotate) return; // dont rotate when rotation is turned off
    
    static bool rotation_locked = false;
    static float angleX = 0, angleY = 0, angleZ = 0;

    if (get_accel(angleX, angleY, angleZ)) {
        int target_rot = -1;

        if (angleX > 0.8 && !rotation_locked) {
            target_rot = 0; // LV_DISP_ROT_NONE
        } else if (angleX < -0.8 && !rotation_locked) {
            target_rot = 2; // LV_DISP_ROT_180
        } else if (angleY < -0.8 && !rotation_locked) {
            target_rot = 1; // LV_DISP_ROT_90
        } else if (angleY > 0.8 && !rotation_locked) {
            target_rot = 3; // LV_DISP_ROT_270
        }

        // If we found a valid rotation, apply it
        if (target_rot != -1) {
            set_software_rotation(target_rot);
            rotation_locked = true;
        }

        // Reset lock when device is flat
        if ((angleX <= 0.8 && angleX >= -0.8) && (angleY <= 0.8 && angleY >= -0.8)) {
            rotation_locked = false;
        }
    }
}


void init_lvgl_interface() {
    lv_init();

    // 1. Buffer Setup (Double Buffering in Fast RAM)
    uint32_t buf_size = LCD_WIDTH * LCD_HEIGHT / 20;
    
    // Allocate Buffer 1
    lv_color_t *buf1 = (lv_color_t *)heap_caps_malloc(buf_size * sizeof(lv_color_t), MALLOC_CAP_DMA);
    // Allocate Buffer 2
    lv_color_t *buf2 = (lv_color_t *)heap_caps_malloc(buf_size * sizeof(lv_color_t), MALLOC_CAP_DMA);

    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, buf_size);

    // 2. Display Driver Setup
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = LCD_WIDTH;
    disp_drv.ver_res = LCD_HEIGHT;
    disp_drv.flush_cb = my_disp_flush;
    
    // Rotation Config
    disp_drv.rounder_cb = example_lvgl_rounder_cb; // Needed for text rotation
    disp_drv.sw_rotate = 1;                        // Enable software rotation
    
    disp_drv.draw_buf = &draw_buf;
    disp_drv.full_refresh = 0;
    lv_disp_drv_register(&disp_drv);

    // 3. Input Driver Setup
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register(&indev_drv);
    
    // 4. Timer Setup
    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback = &example_increase_lvgl_tick, .name = "lvgl_tick"
    };
    esp_timer_handle_t lvgl_tick_timer = NULL;
    esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer);
    esp_timer_start_periodic(lvgl_tick_timer, 2 * 1000);

    lv_disp_set_rotation(NULL, (lv_disp_rot_t)1); // rotate by default that the usb port is a top

    ui_init();

    init_screens_manager();
}

void update_gui() {
    lv_timer_handler();
    handle_screen_rotation();
}

void set_software_rotation(int rotation_code) {
    // 0=0, 1=90, 2=180, 3=270
    lv_disp_set_rotation(NULL, (lv_disp_rot_t)rotation_code);
}

static int activeScreenIndex = -1; // -1 = Home Screen
// static int lastSwitchTime = 0;

// --- NAVIGATION ACTIONS ---

void go_next_screen() {
    Serial.println("going to next screen");

    int lastScreenIndex = get_screen_count() - 1;
    if (activeScreenIndex < lastScreenIndex) {
        load_screen_by_index(activeScreenIndex + 1, false);
        activeScreenIndex++;
    }
}

void go_prev_screen() {
    Serial.println("going to previous screen");

    if (activeScreenIndex > -1) {
        load_screen_by_index(activeScreenIndex - 1, false);
        activeScreenIndex--;
    }
}

void go_back_from_market_data_setting() {
    Serial.println("going back from settings");

    load_screen_by_index(activeScreenIndex, true);
}

void go_to_home_screen() {
    load_screen_by_index(-1, false);
    activeScreenIndex = -1;

    //delay to not accidently click on the menu items
    delay(600);
}

// --- SCREEN LOADER ---

void load_screen_by_index(int index, bool goingFromSettings) {
    //if we would change to the same screen
    if (index == activeScreenIndex && !goingFromSettings) return;

    // handle home screen load
    if (index == -1) {
        lv_scr_load(ui_homeScreen);
        return;
    }

    BaseScreen* screenObj = get_screen_ptr(index); 
    if (screenObj == nullptr) return;

    // select which ui to load
    lv_obj_t* targetScreenUI = nullptr;
    
    switch (screenObj->getType()) {
        case ScreenType::STOCK: targetScreenUI = ui_stockScreen; break;
        case ScreenType::CRYPTO: targetScreenUI = ui_cryptoScreen; break;
        case ScreenType::CLOCK: targetScreenUI = ui_clockScreen; break;
        case ScreenType::TIMER: targetScreenUI = ui_timerScreen; break;
        // case ScreenType::AI_TEXT: targetScreenUI = ui_aiTextScreen; break;
        // Add others...
        default: return;
    }

    //render the correct data on screen
    screenObj->render();

    lv_scr_load(targetScreenUI);
}


void updateScreen() {
    //update current active screen
    updateActiveScreen(activeScreenIndex);
    
    //update all running timers
    int endedTimerScreenIndex = updateAllTimers();
    if(endedTimerScreenIndex != -1) {
        load_screen_by_index(endedTimerScreenIndex, false);
        activeScreenIndex = endedTimerScreenIndex;
    }
}

void updateScreenForce() {
    if (activeScreenIndex == -1) return;

    Serial.println("Swipe down and force data refresh");

    BaseScreen* activeScreen = get_screen_ptr(activeScreenIndex);
    activeScreen->resetLastFetchTime();
}

BaseScreen* get_active_screen() {
    return get_screen_ptr(activeScreenIndex);
}