/**
 * @file scrn_main.cpp
 * @author amagai
 * @brief AP一覧を表示するメインスクリーンの実装
 * @version 0.1
 * @date 2026-05-09
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include <iostream>
#include <string>
#include <sstream>
#include <stdlib.h>
#include <WiFi.h>

#include "scrn_main.h"
#include "screen_id.h"
#include "system_status.h"

LV_FONT_DECLARE(myrica_m_24);


/**
 * @brief ボタンとスワイプ操作のコールバック
 * 
 * @param e イベントオブジェクト
 */
void ScreenMain::callback(lv_event_t *e)
{
    lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        ScreenMain *scrn = static_cast<ScreenMain *>(lv_event_get_user_data(e));
        scrn->on_button(obj);
    }
    else if (code == LV_EVENT_LONG_PRESSED)
    {
        ScreenMain *scrn = static_cast<ScreenMain *>(lv_event_get_user_data(e));
        scrn->on_ap_item_long_press(obj);
    }
    else if (code == LV_EVENT_GESTURE)
    {
        ScreenMain *scrn = static_cast<ScreenMain *>(lv_event_get_user_data(e));
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
        scrn->on_swipe(dir);
    }
}


void ScreenMain::setup()
{
    // Setup code for the main screen
    ScreenBase::setup();

    lv_obj_set_style_bg_color(lv_screen, lv_color_make(0, 0, 0), 0);

    // スキャンボタン
    btn_scan = lv_btn_create(lv_screen);
    lv_obj_set_size(btn_scan, 60, 40);
    lv_obj_align(btn_scan, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_add_event_cb(btn_scan, callback, LV_EVENT_CLICKED, this);
    lv_obj_set_style_bg_color(btn_scan, lv_color_make(28,85,166), 0);

    lv_obj_t *label_scan = lv_label_create(btn_scan);
    lv_label_set_text(label_scan, "Scan!");
    lv_obj_center(label_scan);

    // バッテリー残量ラベル
    label_battery = lv_label_create(lv_screen);
    lv_label_set_text(label_battery, LV_SYMBOL_BATTERY_FULL);
    lv_obj_set_style_text_color(label_battery, lv_color_make(100, 255, 100), 0);
    lv_obj_align(label_battery, LV_ALIGN_TOP_RIGHT, -10, 0);

    // ステータスラベル
    label_status = lv_label_create(lv_screen);
    lv_label_set_text(label_status, "Ready to scan");
    lv_obj_set_style_text_color(label_status, lv_color_make(100, 255, 100), 0);
    lv_obj_align(label_status, LV_ALIGN_TOP_MID, 0, 20);

    // AP一覧のリスト
    list_ap = lv_list_create(lv_screen);
    lv_obj_set_size(list_ap, 300, 180);
    lv_obj_align(list_ap, LV_ALIGN_TOP_MID, 0, 50);
    lv_obj_set_style_bg_color(list_ap, lv_color_make(20, 20, 20), 0);
    lv_obj_set_style_border_color(list_ap, lv_color_make(100, 100, 100), 0);
    lv_obj_set_style_border_width(list_ap, 1, 0);

    // スワイプジェスチャーの有効化
    lv_obj_add_event_cb(lv_screen, callback, LV_EVENT_GESTURE, this);

    scan_in_progress = false;
    scan_start_time = 0;
    prev_battery_level = -1;
}


void ScreenMain::set_battery_level(int level)
{
    if( level < 0 ) 
        level = 0;
    if( level > 100 ) 
        level = 100;
    if( prev_battery_level != level )
    {
        prev_battery_level = level;
        if( prev_battery_level > 80 )
        {
            lv_obj_set_style_text_color(label_battery, lv_color_make(182, 182, 182), 0);
            lv_label_set_text(label_battery, LV_SYMBOL_BATTERY_FULL);
        }
        else if( prev_battery_level > 60 )
        {
            lv_obj_set_style_text_color(label_battery, lv_color_make(182, 182, 182), 0);
            lv_label_set_text(label_battery, LV_SYMBOL_BATTERY_3);
        }
        else if( prev_battery_level > 40 )
        {
            lv_obj_set_style_text_color(label_battery, lv_color_make(182, 182, 182), 0);
            lv_label_set_text(label_battery, LV_SYMBOL_BATTERY_2);
        }
        else if( prev_battery_level > 20 )
        {
            // 20%以下ならオレンジ色で表示
            lv_obj_set_style_text_color(label_battery, lv_color_make(255, 165, 0), 0);
            lv_label_set_text(label_battery, LV_SYMBOL_BATTERY_1);
        }
        else
        {
            // 20%以下なら赤く表示
            lv_obj_set_style_text_color(label_battery, lv_color_make(255, 0, 0), 0);
            lv_label_set_text(label_battery, LV_SYMBOL_BATTERY_EMPTY);
        }
    }
}


void ScreenMain::loop()
{
    // WiFiスキャン完了チェック
    if (scan_in_progress) {
        int scan_result = WiFi.scanComplete();
        if (scan_result >= 0) {
            scan_in_progress = false;
            update_ap_list();
        }
        
        // スキャン開始から30秒経過したらタイムアウト
        if (millis() - scan_start_time > 30000) {
            scan_in_progress = false;
            WiFi.scanDelete();
            lv_label_set_text(label_status, "Scan timeout");
            lv_obj_set_style_text_color(label_status, lv_color_make(255, 100, 100), 0);
        }
    }
    // バッテリー残量更新
    set_battery_level( sys_status.battery_level );
}


void ScreenMain::on_button(lv_obj_t *btn)
{
    if (btn == btn_scan)
    {
        // WiFi scan button pressed
        start_wifi_scan();
    }
}


void ScreenMain::on_swipe(lv_dir_t dir)
{
    if (dir == LV_DIR_LEFT)
    {
        // 左スワイプでシャットダウン画面へ
        change_screen(SCREEN_ID_SHUTDOWN, SCREEN_ANIM_LEFT);
    }
    else if (dir == LV_DIR_RIGHT)
    {
        // 右スワイプでTerminal画面へ
        change_screen(SCREEN_ID_TERMINAL, SCREEN_ANIM_RIGHT);
    }
}


void ScreenMain::start_wifi_scan()
{
    if (scan_in_progress) {
        return; // 既にスキャン中の場合は何もしない
    }
    
    lv_label_set_text(label_status, "Scanning...");
    lv_obj_set_style_text_color(label_status, lv_color_make(255, 255, 100), 0);
    
    // scanボタンを無効化
    lv_obj_add_state(btn_scan, LV_STATE_DISABLED);

    clear_ap_list();
    
    // 非同期スキャンを開始
    WiFi.scanNetworks(true, false, false, 300);
    scan_in_progress = true;
    scan_start_time = millis();
}


void ScreenMain::update_ap_list()
{
    int n = WiFi.scanComplete();
    
    if (n == WIFI_SCAN_FAILED) {
        lv_label_set_text(label_status, "Scan failed");
        lv_obj_set_style_text_color(label_status, lv_color_make(255, 100, 100), 0);
        return;
    }
    
    if (n == 0) {
        lv_label_set_text(label_status, "No networks found");
        lv_obj_set_style_text_color(label_status, lv_color_make(255, 200, 100), 0);
        return;
    }
    
    char status_buf[64];
    snprintf(status_buf, sizeof(status_buf), "Found %d networks", n);
    lv_label_set_text(label_status, status_buf);
    lv_obj_set_style_text_color(label_status, lv_color_make(100, 255, 100), 0);
    
    // AP一覧をリストに追加
    for (int i = 0; i < n; i++) {
        char ap_info[128];
        String ssid = WiFi.SSID(i);
        int32_t rssi = WiFi.RSSI(i);
        wifi_auth_mode_t authMode = WiFi.encryptionType(i);
        
        // SSIDが空の場合は「Hidden Network」と表示
        if (ssid.length() == 0) {
            ssid = "<Hidden Network>";
        }
        
        // 暗号化タイプを文字列に変換
        const char* security;
        switch (authMode) {
            case WIFI_AUTH_OPEN: security = "Open"; break;
            case WIFI_AUTH_WEP: security = "WEP"; break;
            case WIFI_AUTH_WPA_PSK: security = "WPA"; break;
            case WIFI_AUTH_WPA2_PSK: security = "WPA2"; break;
            case WIFI_AUTH_WPA_WPA2_PSK: security = "WPA/WPA2"; break;
            case WIFI_AUTH_WPA2_ENTERPRISE: security = "WPA2-EAP"; break;
            case WIFI_AUTH_WPA3_PSK: security = "WPA3"; break;
            case WIFI_AUTH_WPA2_WPA3_PSK: security = "WPA2/WPA3"; break;
            default: security = "Unknown"; break;
        }
        
        snprintf(ap_info, sizeof(ap_info), "%s (%ddBm) [%s]", 
                ssid.c_str(), rssi, security);
        
        lv_obj_t *btn = lv_list_add_btn(list_ap, LV_SYMBOL_WIFI, ap_info);
        // btnのスタイル設定
        lv_obj_set_style_bg_color(btn, lv_color_make(30, 30, 30), 0);
        
        // 長押しイベントを追加
        lv_obj_add_event_cb(btn, callback, LV_EVENT_LONG_PRESSED, this);
        
        // AP情報をユーザーデータとして保存（シリアル出力用）
        ap_info_t *stored_ap_info = (ap_info_t*)malloc(sizeof(ap_info_t));
        strncpy(stored_ap_info->ssid, ssid.c_str(), sizeof(stored_ap_info->ssid)-1);
        stored_ap_info->ssid[sizeof(stored_ap_info->ssid)-1] = '\0';
        stored_ap_info->channel = WiFi.channel(i);
        memcpy(stored_ap_info->bssid, WiFi.BSSID(i), 6);
        lv_obj_set_user_data(btn, stored_ap_info);
        
        // 電波強度に応じて色を変更
        if (rssi > -60) {
            lv_obj_set_style_text_color(btn, lv_color_make(100, 255, 100), 0); // 強い
        } else if (rssi > -70) {
            lv_obj_set_style_text_color(btn, lv_color_make(255, 255, 100), 0); // 中程度
        } else {
            lv_obj_set_style_text_color(btn, lv_color_make(255, 150, 100), 0); // 弱い
        }
    }
    
    // スキャン結果をクリア
    WiFi.scanDelete();

    // scanボタンを有効化
    lv_obj_clear_state(btn_scan, LV_STATE_DISABLED);
}


void ScreenMain::clear_ap_list()
{
    // リスト内の各アイテムのユーザーデータ（AP情報）を解放
    uint32_t child_count = lv_obj_get_child_cnt(list_ap);
    for (uint32_t i = 0; i < child_count; i++) {
        lv_obj_t *child = lv_obj_get_child(list_ap, i);
        if (child) {
            ap_info_t *user_data = (ap_info_t*)lv_obj_get_user_data(child);
            if (user_data) {
                free(user_data);
            }
        }
    }
    
    lv_obj_clean(list_ap);
}


void ScreenMain::on_ap_item_long_press(lv_obj_t *btn)
{
    ap_info_t *ap_info = (ap_info_t*)lv_obj_get_user_data(btn);
    if (ap_info) {
        Serial.println("=== AP Item Long Pressed ===");
        Serial.print("BSSID: ");
        for (int i = 0; i < 6; i++) {
            Serial.print(ap_info->bssid[i], HEX);
            if (i < 5) Serial.print(":");
        }
        Serial.println();
        Serial.print("SSID: ");
        Serial.println(ap_info->ssid);
        Serial.print("Channel: ");
        Serial.println(ap_info->channel);
        Serial.println("=============================");
        // 詳細画面へ遷移
        // sys_statusに選択中のAP情報を保存
        memcpy(sys_status.target_ap.bssid, ap_info->bssid, 6);
        strncpy(sys_status.target_ap.ssid, ap_info->ssid, sizeof(sys_status.target_ap.ssid)-1);
        sys_status.target_ap.ssid[sizeof(sys_status.target_ap.ssid)-1] = '\0';
        sys_status.target_ap.channel = ap_info->channel;
        change_screen(SCREEN_ID_AP_DETAIL, SCREEN_ANIM_LEFT);
    }
}
