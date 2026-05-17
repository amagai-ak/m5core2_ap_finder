/**
 * @file scrn_ap_detail.cpp
 * @author amagai
 * @brief AP詳細画面の実装
 * @version 0.1
 * @date 2026-05-17
 * 
 * @copyright Copyright (c) 2026
 * 
 * フレームの種別毎にカウントはしているが，現状は画面に表示していない．
 */
#include <M5Unified.h>
#include "scrn_ap_detail.h"
#include "screen_id.h"
#include "system_status.h"

#include "esp_wifi.h"
#include "esp_wifi_types.h"

#include "ouitable.h"

LV_FONT_DECLARE(font_notosansmono_48);



/**
 * @brief RSSIをプロットするクラスのコンストラクタ
 * 
 */
RSSITSPlotter::RSSITSPlotter()
{
    canvas_buf = nullptr;
    rssi_range_min = RSSI_MIN;
    rssi_range_max = RSSI_MAX;
}


/**
 * @brief RSSIをプロットするクラスのデストラクタ
 * 
 */
RSSITSPlotter::~RSSITSPlotter()
{
    if (canvas_buf) {
        delete[] canvas_buf;
        canvas_buf = nullptr;
    }
}


/**
 * @brief 値のクリア
 * 
 */
void RSSITSPlotter::clear()
{
    for (int i = 0; i < MAX_POINTS; i++) {
        rssi_points[i] = RSSI_INVALID;
    }
    rssi_range_min = RSSI_MIN;
    rssi_range_max = RSSI_MAX;
}


/**
 * @brief 描画．200ms毎に呼び出される
 * 
 */
void RSSITSPlotter::draw()
{
    // キャンバスの描画
    lv_layer_t layer;
    lv_canvas_init_layer(canvas, &layer);
    lv_canvas_fill_bg(canvas, lv_color_make(0x00, 0x00, 0x40), LV_OPA_COVER);

    lv_draw_arc_dsc_t point_dsc;
    lv_draw_arc_dsc_init(&point_dsc);
    point_dsc.color = lv_color_make(255, 255, 0);
    point_dsc.width = 1;
    for (int i = 0; i < MAX_POINTS; i++) {
        int rssi = rssi_points[i];
        if( rssi == RSSI_INVALID)
            continue;
        if (rssi < RSSI_MIN) rssi = RSSI_MIN;
        if (rssi > RSSI_MAX) rssi = RSSI_MAX;

        int x = map(i, 0, MAX_POINTS - 1, 0, IMG_W - 1);
        int y = map(rssi, rssi_range_min, rssi_range_max, IMG_H - 1, 0);
        point_dsc.center.x = x;
        point_dsc.center.y = y;
        point_dsc.radius = 2;
        point_dsc.start_angle = 0;
        point_dsc.end_angle = 360;
        lv_draw_arc(&layer, &point_dsc);
    }
    lv_canvas_finish_layer(canvas, &layer);

    // データをシフトする
    for (int i = MAX_POINTS - 1; i > 0; i--) {
        rssi_points[i] = rssi_points[i - 1];
    }
    rssi_points[0] = RSSI_INVALID;
}


void RSSITSPlotter::init(lv_obj_t *p, int x, int y)
{
    parent = p;
    canvas = lv_canvas_create(parent);
    canvas_buf = new uint8_t[LV_CANVAS_BUF_SIZE(IMG_W, IMG_H, 32, LV_DRAW_BUF_STRIDE_ALIGN)];
    lv_canvas_set_buffer(canvas, canvas_buf, IMG_W, IMG_H, LV_COLOR_FORMAT_NATIVE);
    lv_obj_align(canvas, LV_ALIGN_TOP_LEFT, x, y);
    lv_obj_set_style_border_width(canvas, 0, 0);
    clear();

    // labelの作成
    int font_h = 14;
    label_range_max = lv_label_create(canvas);
    lv_label_set_text(label_range_max, "-30");
    lv_obj_set_style_text_color(label_range_max, lv_color_make(255, 255, 255), 0);
    lv_obj_align(label_range_max, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_text_font(label_range_max, &lv_font_montserrat_14, 0);
    label_range_min = lv_label_create(canvas);
    lv_label_set_text(label_range_min, "-90");
    lv_obj_set_style_text_color(label_range_min, lv_color_make(255, 255, 255), 0);
    lv_obj_align(label_range_min, LV_ALIGN_TOP_LEFT, 0, IMG_H - font_h);
    lv_obj_set_style_text_font(label_range_min, &lv_font_montserrat_14, 0);

    // timer
    lv_timer_create([](lv_timer_t *timer){
        RSSITSPlotter *plt = static_cast<RSSITSPlotter *>(lv_timer_get_user_data(timer));
        plt->draw();
    }, UPDATE_INTERVAL_MS, this);
}


/**
 * @brief RSSI値の設定
 * 
 * @param rssi RSSI値
 */
void RSSITSPlotter::set_rssi(int rssi)
{
    rssi_points[0] = rssi;
}



/**
 * @brief コンストラクタ
 * 
 */
RSSIPolarPlotter::RSSIPolarPlotter()
{
    canvas_buf = nullptr;
    clear();
}


RSSIPolarPlotter::~RSSIPolarPlotter()
{
    if (canvas_buf) {
        delete[] canvas_buf;
        canvas_buf = nullptr;
    }
}


/**
 * @brief 初期化
 * 
 * @param p 親オブジェクト
 * @param x X座標
 * @param y Y座標
 */
void RSSIPolarPlotter::init(lv_obj_t *p, int x, int y)
{
    parent = p;
    canvas = lv_canvas_create(parent);
    canvas_buf = new uint8_t[LV_CANVAS_BUF_SIZE(IMG_W, IMG_H, 32, LV_DRAW_BUF_STRIDE_ALIGN)];
    lv_canvas_set_buffer(canvas, canvas_buf, IMG_W, IMG_H, LV_COLOR_FORMAT_NATIVE);
    lv_obj_align(canvas, LV_ALIGN_TOP_LEFT, x, y);
    lv_obj_set_style_border_width(canvas, 0, 0);

    // ラベルの作成
    label_range_max = lv_label_create(parent);
    lv_label_set_text(label_range_max, "-30");
    lv_obj_set_style_text_color(label_range_max, lv_color_make(255, 255, 255), 0);
    lv_obj_align(label_range_max, LV_ALIGN_TOP_LEFT, x, y + IMG_H / 2);
    lv_obj_set_style_text_font(label_range_max, &lv_font_montserrat_14, 0);

    label_range_min = lv_label_create(parent);
    lv_label_set_text(label_range_min, "-90");
    lv_obj_set_style_text_color(label_range_min, lv_color_make(255, 255, 255), 0);
    lv_obj_align(label_range_min, LV_ALIGN_TOP_LEFT, x + IMG_W / 2 - 20, y + IMG_H / 2);
    lv_obj_set_style_text_font(label_range_min, &lv_font_montserrat_14, 0);

    current_angle = 0;
    clear();
    draw();

    // timer
    lv_timer_create([](lv_timer_t *timer){
        RSSIPolarPlotter *plt = static_cast<RSSIPolarPlotter *>(lv_timer_get_user_data(timer));
        plt->draw();
    }, UPDATE_INTERVAL_MS, this);

}


/**
 * @brief RSSI値の設定
 * 
 * @param angle 角度 (0-359)
 * @param rssi RSSI値
 */
void RSSIPolarPlotter::set_rssi(int angle, int rssi)
{
    if (angle < 0 || angle >= 360) return;
    int index = angle / 10;
    if (index < 0) index = 0;
    if (index > 35) index = 35;

    if (rssi_angle_min[index] == -100 || rssi < rssi_angle_min[index]) 
        rssi_angle_min[index] = rssi;
    if (rssi > rssi_angle_max[index])
        rssi_angle_max[index] = rssi;
}


/**
 * @brief RSSI値のクリア
 * 
 */
void RSSIPolarPlotter::clear()
{
    for (int i = 0; i < 36; i++) {
        rssi_angle_max[i] = RSSI_INVALID;
        rssi_angle_min[i] = RSSI_INVALID;
    }
    current_angle = 0;
    rssi_range_max = RSSI_MAX;
    rssi_range_min = RSSI_MIN;
}


/**
 * @brief RSSI値の範囲を計算する
 * 
 * @param r_min 最小RSSI値
 * @param r_max 最大RSSI値
 */
void RSSIPolarPlotter::calc_range(int &r_min, int &r_max)
{
    r_min = RSSI_MAX;
    r_max = RSSI_MIN;
    for (int i = 0; i < 36; i++) {
        if (rssi_angle_min[i] != RSSI_INVALID) {
            if (rssi_angle_min[i] < r_min) r_min = rssi_angle_min[i];
        }
        if (rssi_angle_max[i] != RSSI_INVALID) {
            if (rssi_angle_max[i] > r_max) r_max = rssi_angle_max[i];
        }
    }
    if (r_min == RSSI_MAX) r_min = RSSI_MIN;
    if (r_max == RSSI_MIN) r_max = RSSI_MAX;

    // 値は10の倍数に丸める
    r_min = ((r_min - 9) / 10) * 10;
    r_max = ((r_max + 9) / 10) * 10;

    sys_status.debug1 = r_min;
    sys_status.debug2 = r_max;
}


/**
 * @brief RSSI値を半径に変換する
 * 
 * @param rssi 
 * @return int 
 */
int RSSIPolarPlotter::rssi_to_radius(int rssi)
{
    if (rssi < rssi_range_min) rssi = rssi_range_min;
    if (rssi > rssi_range_max) rssi = rssi_range_max;
    int radius = map(rssi, rssi_range_min, rssi_range_max, 10, IMG_W / 2 - 10);
    return radius;
}


/**
 * @brief RSSI値を座標に変換する
 * 
 * @param angle 角度 (0-359)
 * @param rssi RSSI値
 * @param x X座標
 * @param y Y座標
 */
void RSSIPolarPlotter::rssi_to_xy(int angle, int rssi, int &x, int &y)
{
    int center_x = IMG_W / 2;
    int center_y = IMG_H / 2;
    int radius = rssi_to_radius(rssi);

    float angle_rad = (angle + 90) * 3.14159 / 180.0;
    x = center_x + radius * cos(angle_rad);
    y = center_y - radius * sin(angle_rad);
}


/**
 * @brief RSSIプロットを描画する
 * 
 */
void RSSIPolarPlotter::draw()
{
    int x,y;
    char buf[16];

    calc_range(rssi_range_min, rssi_range_max);
    snprintf(buf, sizeof(buf), "%d", rssi_range_max);
    lv_label_set_text(label_range_max, buf);

    snprintf(buf, sizeof(buf), "%d", rssi_range_min);
    lv_label_set_text(label_range_min, buf);

    // キャンバスの描画
    lv_layer_t layer;
    lv_canvas_init_layer(canvas, &layer);
    lv_canvas_fill_bg(canvas, lv_color_make(0x00, 0x00, 0x40), LV_OPA_COVER); // キャンバスの背景を黒に設定

    int center_x = IMG_W / 2;
    int center_y = IMG_H / 2;
    int radius = IMG_W / 2 - 10;

    // 外枠と同心円を描画
    lv_draw_arc_dsc_t arc_dsc;
    lv_draw_arc_dsc_init(&arc_dsc);
    arc_dsc.color = lv_color_make(0x80, 0x80, 0x80); // 線の色をグレーに設定
    arc_dsc.width = 1; // 線の太さを1ピクセルに設定
    arc_dsc.center.x = IMG_W / 2;
    arc_dsc.center.y = IMG_H / 2;
    arc_dsc.radius = RADIUS_MIN;
    arc_dsc.start_angle = 0;
    arc_dsc.end_angle = 360;
    arc_dsc.width = 1;
    lv_draw_arc(&layer, &arc_dsc);
    arc_dsc.radius = RADIUS_MAX;
    lv_draw_arc(&layer, &arc_dsc);

    // 十字線を描画
    lv_draw_line_dsc_t line_dsc;
    lv_draw_line_dsc_init(&line_dsc);
    line_dsc.color = lv_color_make(0x80, 0x80, 0x80); // 線の色をグレーに設定
    line_dsc.width = 1;
    line_dsc.round_start = 1;
    line_dsc.round_end = 1;
    line_dsc.p1.x = 0;
    line_dsc.p1.y = center_y;
    line_dsc.p2.x = IMG_W;
    line_dsc.p2.y = center_y;
    lv_draw_line(&layer, &line_dsc);
    line_dsc.p1.x = center_x;
    line_dsc.p1.y = 0;
    line_dsc.p2.x = center_x;
    line_dsc.p2.y = IMG_H;
    lv_draw_line(&layer, &line_dsc);

    // 現在の角度を示す小さな円を描画
    float angle_rad = (current_angle + 90) * 3.14159 / 180.0;
    x = center_x + radius * cos(angle_rad);
    y = center_y - radius * sin(angle_rad);
    lv_draw_arc_dsc_t small_arc_dsc;
    lv_draw_arc_dsc_init(&small_arc_dsc);
    small_arc_dsc.color = lv_color_make(255, 0, 0);
    small_arc_dsc.width = 1;
    small_arc_dsc.center.x = x;
    small_arc_dsc.center.y = y;
    small_arc_dsc.radius = 5;
    small_arc_dsc.start_angle = 0;
    small_arc_dsc.end_angle = 360;
    lv_draw_arc(&layer, &small_arc_dsc);

    // RSSIプロットを描画
    line_dsc.width = 2;
    line_dsc.round_start = 1;
    line_dsc.round_end = 1;

    line_dsc.color = lv_color_make(64, 128, 64);
    for (int i = 0; i < 37; i++) {
        int idx = (i % 36);
        int rssi = rssi_angle_min[idx];
        if (rssi < -100) rssi = -100;
        if (rssi > 0) rssi = 0;
        int r_base = 10;
        float angle_rad = (i * 10 + 90) * 3.14159 / 180.0;
        rssi_to_xy(i * 10, rssi, x, y);
        line_dsc.p1.x = line_dsc.p2.x;
        line_dsc.p1.y = line_dsc.p2.y;
        line_dsc.p2.x = x;
        line_dsc.p2.y = y;
        if( i > 0 )
            lv_draw_line(&layer, &line_dsc);
    }

    line_dsc.color = lv_color_make(100, 255, 100);
    for (int i = 0; i < 37; i++) {
        int idx = (i % 36);
        int rssi = rssi_angle_max[idx];
        if (rssi < RSSI_MIN) rssi = RSSI_MIN;
        if (rssi > 0) rssi = 0;
        int r_base = 10;
        float angle_rad = (i * 10 + 90) * 3.14159 / 180.0;
        rssi_to_xy(i * 10, rssi, x, y);
        line_dsc.p1.x = line_dsc.p2.x;
        line_dsc.p1.y = line_dsc.p2.y;
        line_dsc.p2.x = x;
        line_dsc.p2.y = y;
        if( i > 0 )
            lv_draw_line(&layer, &line_dsc);
    }

    lv_canvas_finish_layer(canvas, &layer);
}


void ScreenAPDetail::callback(lv_event_t *e)
{
    lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED)
    {
        ScreenAPDetail *scrn = static_cast<ScreenAPDetail *>(lv_event_get_user_data(e));
        scrn->on_button(obj);
    }
    else if (code == LV_EVENT_GESTURE)
    {
        ScreenAPDetail *scrn = static_cast<ScreenAPDetail *>(lv_event_get_user_data(e));
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
        scrn->on_swipe(dir);
    }
}


void ScreenAPDetail::on_button(lv_obj_t *btn)
{
    if (btn == btn_clear)
    {
        // Clear button pressed
        rssi_plotter.clear();
        rssi_plotter.draw();
        angle_z = 0;

        // 現在のIMUの値をオフセットに加算
        sys_status.imu_gz_offset += sys_status.imu_gz;
    }
}


/**
 * @brief AP詳細画面のセットアップ
 * 
 */
void ScreenAPDetail::setup()
{
    // Setup code for the AP detail screen
    ScreenBase::setup();

    int font_h = 24; // フォントの高さ（適宜調整）
    int lbl_ypos = 0;

    lv_obj_set_style_bg_color(lv_screen, lv_color_make(0, 0, 0), 0);
    lv_obj_clear_flag(lv_screen, LV_OBJ_FLAG_SCROLLABLE);

    // ラベルの作成
    lbl_ypos = 0;
    label_ssid = lv_label_create(lv_screen);
    lv_label_set_text(label_ssid, "SSID: ");
    lv_obj_set_style_text_color(label_ssid, lv_color_make(255, 255, 255), 0);
    lv_obj_align(label_ssid, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_text_font(label_ssid, &lv_font_montserrat_18, 0);

    lbl_ypos += font_h;
    label_bssid = lv_label_create(lv_screen);
    lv_label_set_text(label_bssid, "BSSID: ");
    lv_obj_set_style_text_color(label_bssid, lv_color_make(255, 255, 255), 0);
    lv_obj_align(label_bssid, LV_ALIGN_TOP_LEFT, 0, lbl_ypos);
    lv_obj_set_style_text_font(label_bssid, &lv_font_montserrat_18, 0);

    // ベンダ名のラベルを作成
    // ベンダ名は長くなる場合があるが，スクロールはせず，画面端で切れるようにする．
    lbl_ypos += font_h;
    label_vendor = lv_label_create(lv_screen);
    lv_label_set_text(label_vendor, "Vendor: ");
    lv_obj_set_style_text_color(label_vendor, lv_color_make(255, 255, 255), 0);
    lv_obj_align(label_vendor, LV_ALIGN_TOP_LEFT, 0, lbl_ypos);
    lv_obj_set_style_text_font(label_vendor, &lv_font_montserrat_18, 0);
    lv_label_set_long_mode(label_vendor, LV_LABEL_LONG_CLIP);

    lbl_ypos += font_h;
    label_channel = lv_label_create(lv_screen);
    lv_label_set_text(label_channel, "Channel: ");
    lv_obj_set_style_text_color(label_channel, lv_color_make(255, 255, 255), 0);
    lv_obj_align(label_channel, LV_ALIGN_TOP_LEFT, 0, lbl_ypos);
    lv_obj_set_style_text_font(label_channel, &lv_font_montserrat_18, 0);

    // RSSIラベルの作成
    lbl_ypos += font_h;
    label_rssi = lv_label_create(lv_screen);
    lv_label_set_text(label_rssi, "RSSI: ");
    lv_obj_set_style_text_color(label_rssi, lv_color_make(255, 255, 255), 0);
    lv_obj_align(label_rssi, LV_ALIGN_TOP_LEFT, 0, lbl_ypos);
    lv_obj_set_style_text_font(label_rssi, &font_notosansmono_48, 0);

    // dBmラベルの作成. RSSIラベルの右側下に配置．
    label_dbm = lv_label_create(lv_screen);
    lv_label_set_text(label_dbm, "dBm");
    lv_obj_set_style_text_color(label_dbm, lv_color_make(255, 255, 255), 0);
    lv_obj_align(label_dbm, LV_ALIGN_TOP_LEFT, 100, lbl_ypos + 22);
    lv_obj_set_style_text_font(label_dbm, &lv_font_montserrat_18, 0);

    // Clearボタンの作成
    btn_clear = lv_btn_create(lv_screen);
    lv_obj_set_size(btn_clear, 80, 32);
    lv_obj_align(btn_clear, LV_ALIGN_TOP_LEFT, 220, font_h * 3 + 120 + 2);
    lv_obj_add_event_cb(btn_clear, callback, LV_EVENT_CLICKED, this);
    lv_obj_t *label_clear = lv_label_create(btn_clear);
    lv_label_set_text(label_clear, "Clear");
    lv_obj_center(label_clear);
    // ボタンは少し暗い青色に設定
    lv_obj_set_style_bg_color(btn_clear, lv_color_make(28,85,166), 0);


    rssi_plotter.init(lv_screen, 200, font_h * 3);
    rssi_ts_plotter.init(lv_screen, 0, 140);

    // スワイプジェスチャーの有効化
    lv_obj_add_event_cb(lv_screen, callback, LV_EVENT_GESTURE, this);
}

typedef struct __packed {
	uint8_t frame_ctrl[2];
	uint8_t duration_id[2];
	uint8_t addr1[6];
	uint8_t addr2[6];
	uint8_t addr3[6];
	uint8_t sequence_ctrl[2];
	uint8_t addr4[6];
} wifi_ieee80211_mac_hdr_t;

typedef struct {
	wifi_ieee80211_mac_hdr_t hdr;
	uint8_t payload[0]; /* network data ended with 4 bytes csum (CRC32) */
} wifi_ieee80211_packet_t;


/**
 * @brief Wi-Fiパケットタイプを文字列に変換する
 * 
 * @param type パケットタイプ
 * @return const char* パケットタイプの文字列表現
 */
const char * wifi_sniffer_packet_type2str(wifi_promiscuous_pkt_type_t type)
{
    switch (type)
    {
    case WIFI_PKT_MGMT:
        return "MGMT";
    case WIFI_PKT_CTRL:
        return "CTRL";
    case WIFI_PKT_DATA:
        return "DATA";
    case WIFI_PKT_MISC:
        return "MISC";
    default:
        return "UNKNOWN";
    }
}


/**
 * @brief Wi-Fiパケットを処理する
 * 
 * @param buff パケットデータ
 * @param type パケットタイプ
 */
void wifi_sniffer_packet_handler(void* buff, wifi_promiscuous_pkt_type_t type)
{
    int rssi;
    int noise;

    const wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t *)buff;
    const wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)ppkt->payload;
    const wifi_ieee80211_mac_hdr_t *hdr = &ipkt->hdr;

    uint8_t type_subtype = hdr->frame_ctrl[0];
    uint8_t frame_type = (type_subtype >> 2) & 0x03;
    uint8_t frame_subtype = (type_subtype >> 4) & 0x0F;
    rssi = ppkt->rx_ctrl.rssi;
    noise = ppkt->rx_ctrl.noise_floor;
    if (frame_type == 0)    // Management frame
    {
        // BSSIDが一致しない場合は無視
        if (memcmp(hdr->addr3, sys_status.target_ap.bssid, 6) != 0)
            return;

        sys_status.target_ap.framecount_management++;

        // Beaconフレーム(0x80)のみ処理
        if (type_subtype != 0x80 )
            return;
        // BSSIDを取得
        char bssid[18];
        snprintf(bssid, sizeof(bssid), "%02X:%02X:%02X:%02X:%02X:%02X",
                hdr->addr3[0], hdr->addr3[1], hdr->addr3[2],
                hdr->addr3[3], hdr->addr3[4], hdr->addr3[5]);

        // シリアルモニタに出力
        Serial.printf("Packet Type: %s, BSSID: %s, RSSI: %d, FrameType: %d, FrameSubtype: %d, NF: %d\r\n",
            wifi_sniffer_packet_type2str(type), bssid, rssi, frame_type, frame_subtype, noise);

        // TAがターゲットと一致する場合、sys_status.target_apのRSSIを更新
        if (memcmp(hdr->addr2, sys_status.target_ap.bssid, 6) == 0) {
            sys_status.target_ap.rssi = rssi;
            sys_status.target_ap.rssi_timestamp = millis();
        }
    }
    else if (frame_type == 1)   // Control frame
    {
        // BSSIDが一致しない場合は無視
        if (memcmp(hdr->addr1, sys_status.target_ap.bssid, 6) != 0 &&
            memcmp(hdr->addr2, sys_status.target_ap.bssid, 6) != 0 &&
            memcmp(hdr->addr3, sys_status.target_ap.bssid, 6) != 0)
            return;

        sys_status.target_ap.framecount_control++;
    }
    else if (frame_type == 2)   // Data frame
    {
        // BSSIDが一致しない場合は無視
        if (memcmp(hdr->addr1, sys_status.target_ap.bssid, 6) != 0 &&
            memcmp(hdr->addr2, sys_status.target_ap.bssid, 6) != 0 &&
            memcmp(hdr->addr3, sys_status.target_ap.bssid, 6) != 0)
            return;

        sys_status.target_ap.framecount_data++;
    }
}


/**
 * @brief BSSIDからベンダ名を取得する
 * 
 * @param bssid 
 * @return const char* 
 */
const char *ScreenAPDetail::get_vendor_name(const uint8_t *bssid)
{
    uint32_t prefix = (bssid[0] << 16) | (bssid[1] << 8) | bssid[2];
    for (size_t i = 0; i < sizeof(oui_table) / sizeof(oui_entry_t); i++) {
        if (oui_table[i].prefix == prefix) {
            return oui_table[i].vendor;
        }
    }
    return "-";
}


/**
 * @brief 詳細画面が表示される直前に呼ばれる関数
 * 
 * @return int 
 */
int ScreenAPDetail::on_load()
{
    // 画面が表示される直前に呼ばれる
    // sys_status.target_apから情報を取得してラベルに表示
    char buf[64];
    snprintf(buf, sizeof(buf), "SSID: %s", sys_status.target_ap.ssid);
    lv_label_set_text(label_ssid, buf);

    snprintf(buf, sizeof(buf), "BSSID: %02X:%02X:%02X:%02X:%02X:%02X",
             sys_status.target_ap.bssid[0], sys_status.target_ap.bssid[1],
             sys_status.target_ap.bssid[2], sys_status.target_ap.bssid[3],
             sys_status.target_ap.bssid[4], sys_status.target_ap.bssid[5]);
    lv_label_set_text(label_bssid, buf);

    snprintf(buf, sizeof(buf), "Channel: %d", sys_status.target_ap.channel);
    lv_label_set_text(label_channel, buf);

    lv_label_set_text(label_rssi, "?");

    snprintf(buf, sizeof(buf), "Vendor: %s", get_vendor_name(sys_status.target_ap.bssid));
    lv_label_set_text(label_vendor, buf);

    sys_status.target_ap.framecount_management = 0;
    sys_status.target_ap.framecount_control = 0;
    sys_status.target_ap.framecount_data = 0;

    // パケットキャプチャ開始
    esp_wifi_set_channel(sys_status.target_ap.channel, WIFI_SECOND_CHAN_NONE);
    esp_wifi_set_promiscuous_rx_cb(&wifi_sniffer_packet_handler);
    esp_wifi_set_promiscuous(true);

    angle_z = 0.0f;
    prev_imu_time = 0;
    prev_rot_z = 0;
    prev_rssi_update_time = 0;
    prev_rssi_timestamp = 0;

    rssi_plotter.clear();
    rssi_plotter.draw();

    rssi_ts_plotter.clear();
    rssi_ts_plotter.draw();
    return 0;
}


/**
 * @brief 詳細画面が非表示になる直前に呼ばれる関数
 * 
 * @return int 
 */
int ScreenAPDetail::on_unload()
{
    // パケットキャプチャ停止
    esp_wifi_set_promiscuous(false);
    rssi_ts_plotter.clear();
    return 0;
}


void ScreenAPDetail::loop()
{
    float gz;
    uint32_t imu_time;

    imu_time = sys_status.imu_gz_timestamp;
    gz = sys_status.imu_gz;
    uint32_t dt = imu_time - prev_imu_time;
    if (dt > 200) dt = 0; // 大きな時間差は無視
    if (dt > 0)
    {
        angle_z += (gz + prev_rot_z) * (imu_time - prev_imu_time) / 1000.0f / 2.0f; // Z軸回転角度を更新
        prev_imu_time = imu_time;
        prev_rot_z = gz;
        // angle_zを0～+360度の範囲に収める
        if (angle_z < 0) 
            angle_z += 360.0f;
        if (angle_z >= 360.0f) 
            angle_z -= 360.0f;
        rssi_plotter.set_angle((int)angle_z);
    }
    else
    {
        prev_rot_z = gz;
        prev_imu_time = imu_time;
    }

    if( prev_rssi_timestamp != sys_status.target_ap.rssi_timestamp )
    {
        // RSSI値が更新された場合、プロットに記録
        rssi_ts_plotter.set_rssi(sys_status.target_ap.rssi);
        prev_rssi_timestamp = sys_status.target_ap.rssi_timestamp;
    }

    if( sys_status.target_ap.rssi != prev_rssi )
    {
        char buf[32];
        snprintf(buf, sizeof(buf), "%d", sys_status.target_ap.rssi);
        lv_label_set_text(label_rssi, buf);
        prev_rssi = sys_status.target_ap.rssi;
    }
    else
    {
        // RSSIが変化していない場合，タイムスタンプを確認し，一定以上経過していたら「-」表示にする
        if( (millis() - sys_status.target_ap.rssi_timestamp) > 2000 )
        {
            lv_label_set_text(label_rssi, "---");
            rssi_ts_plotter.set_rssi(RSSITSPlotter::RSSI_INVALID);
        }
    }

    if( sys_status.target_ap.rssi_timestamp != prev_rssi_update_time )
    {
        // RSSI値を記録
        rssi_plotter.set_rssi((int)angle_z, sys_status.target_ap.rssi);
        prev_rssi_update_time = sys_status.target_ap.rssi_timestamp;
    }

}


void ScreenAPDetail::on_swipe(lv_dir_t dir)
{
    if (dir == LV_DIR_RIGHT)
    {
        // 右スワイプでメイン画面へ
        change_screen(SCREEN_ID_MAIN, SCREEN_ANIM_RIGHT);
    }
}
