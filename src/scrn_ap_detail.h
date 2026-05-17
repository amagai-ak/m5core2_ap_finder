
#ifndef SCRN_AP_DETAIL_H
#define SCRN_AP_DETAIL_H

#include "screen_base.h"
#include "screen_id.h"


/**
 * @brief RSSIを角度ごとにプロットするクラス
 * 
 */
class RSSIPolarPlotter
{
protected:
    int rssi_angle_min[36]; // 0～35: 0度～350
    int rssi_angle_max[36]; // 0～35: 0度～350
    uint8_t *canvas_buf;
    lv_obj_t *canvas;
    static const int UPDATE_INTERVAL_MS = 200;
    static const int RSSI_INVALID = -100;
    static const int IMG_H = 120;
    static const int IMG_W = 120;
    static const int RSSI_MIN = -90;
    static const int RSSI_MAX = -30;
    static const int RADIUS_MIN = 10;
    static const int RADIUS_MAX = IMG_W / 2 - 10;
    lv_obj_t *parent;
    lv_obj_t *label_range_min;
    lv_obj_t *label_range_max;
    int current_angle;
    int rssi_range_min;
    int rssi_range_max;

    void rssi_to_xy(int angle, int rssi, int &x, int &y);
    int rssi_to_radius(int rssi);
    void calc_range(int &r_min, int &r_max);

public:
    RSSIPolarPlotter();
    ~RSSIPolarPlotter();
    void set_rssi(int angle, int rssi);
    void init(lv_obj_t *p, int x, int y);
    void clear();
    void draw();
    void set_angle(int angle){ current_angle = angle; }
};



/**
 * @brief RSSIを時系列でプロットするクラス
 * 
 */
class RSSITSPlotter
{
protected:
    uint8_t *canvas_buf;
    lv_obj_t *canvas;
    static const int RSSI_MIN = -90;
    static const int RSSI_MAX = -30;
    static const int IMG_H = 100;
    static const int IMG_W = 160;
    static const int MAX_POINTS = 100;
    static const int UPDATE_INTERVAL_MS = 200;
    int rssi_range_min;
    int rssi_range_max;
    int rssi_points[MAX_POINTS];
    lv_obj_t *parent;
    lv_obj_t *label_range_min;
    lv_obj_t *label_range_max;

public:
    static const int RSSI_INVALID = -100;
    RSSITSPlotter();
    ~RSSITSPlotter();
    void init(lv_obj_t *p, int x, int y);
    void set_rssi(int rssi);
    void clear();
    void draw();
};



/**
 * @brief APの詳細情報を表示するスクリーン
 * 
 */
class ScreenAPDetail : public ScreenBase
{
protected:
    lv_obj_t *label_ssid;
    lv_obj_t *label_bssid;
    lv_obj_t *label_rssi;
    lv_obj_t *label_dbm;
    lv_obj_t *label_channel;
    lv_obj_t *label_vendor;
    lv_obj_t *label_angle;
    lv_obj_t *btn_clear;

    int prev_rssi;
    float angle_z;
    float prev_rot_z;
    uint32_t prev_imu_time;
    uint32_t prev_loop_time;
    uint32_t prev_rssi_update_time;
    uint32_t prev_rssi_timestamp;
    RSSIPolarPlotter rssi_plotter;
    RSSITSPlotter rssi_ts_plotter;

    const char *get_vendor_name(const uint8_t *bssid);

public:
    void setup();
    void loop();
    static void callback(lv_event_t *e);
    void on_button(lv_obj_t *btn);
    void on_swipe(lv_dir_t dir);
    int on_load();
    int on_unload();
};

#endif // SCRN_AP_DETAIL_H
