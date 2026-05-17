
#ifndef SCRN_MAIN_H
#define SCRN_MAIN_H

#include "screen_base.h"
#include <WiFi.h>

class ScreenMain : public ScreenBase
{
protected:
    lv_obj_t *btn_scan;
    lv_obj_t *list_ap;
    lv_obj_t *label_status;
    lv_obj_t *label_battery;
    int prev_battery_level;

public:
    bool scan_in_progress;
    unsigned long scan_start_time;
    void setup();
    void loop();
    static void callback(lv_event_t *e);
    void on_button(lv_obj_t *btn);
    void on_swipe(lv_dir_t dir);
    void start_wifi_scan();
    void update_ap_list();
    void clear_ap_list();
    void on_ap_item_long_press(lv_obj_t *btn);
    void set_battery_level(int level);
};

#endif // SCRN_MAIN_H
