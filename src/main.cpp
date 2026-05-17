/**
 * @file main.cpp
 * @author amagai
 * @brief AP Finder for M5Stack Core2 V1.1
 * @version 0.1
 * @date 2026-05-09
 * 
 */

#include <Arduino.h>
#include <M5Unified.h>
#include <WiFi.h>
#include <time.h>

// #define LV_CONF_INCLUDE_SIMPLE
#include <lvgl.h>

#include "lvgl_setup.h"

#include "scrn_main.h"
#include "scrn_shutdown.h"
#include "scrn_terminal.h"
#include "scrn_ap_detail.h"
#include "screen_id.h"

#include "system_status.h"

static const char* time_zone  = "JST-9";

// 各スクリーンのインスタンスを生成
static ScreenMain scrn_main;
static ScreenShutdown scrn_shutdown;
static ScreenTerminal scrn_terminal;
static ScreenAPDetail scrn_ap_detail;

// スクリーンマネージャのインスタンスを生成
static ScreenManager scrn_manager;

// システムステータス
system_status_t sys_status;


/**
 * @brief IMUのZ軸ジャイロ値を更新する関数
 * 
 */
void update_imu_gz()
{
    float gx, gy, gz;

    auto imu_update = M5.Imu.update();
    if (imu_update)
    {
        M5.Imu.getGyroData(&gx, &gy, &gz);
        gz -= sys_status.imu_gz_offset; // オフセットを引く
        sys_status.imu_gz = sys_status.imu_gz * 0.9f + gz * 0.1f; // 簡易ローパスフィルタ
        sys_status.imu_gz_timestamp = millis();
    }
}


/**
 * @brief システム初期化
 * 
 */
void setup() 
{
    auto cfg = M5.config();
    M5.begin(cfg);

    // シリアル，時計は今は使っていないが，デバッグ用に初期化しておく
    Serial.setRxBufferSize(1024);
    Serial.begin(115200);
    setenv("TZ", time_zone, 1);
    tzset();

    // システムステータス初期化
    memset(&sys_status.target_ap, 0, sizeof(sys_status.target_ap));
    sys_status.battery_level = M5.Power.getBatteryLevel();

    // LVGLの初期化
    lvgl_setup();

    // 各スクリーンのセットアップ
    scrn_main.setup();
    scrn_shutdown.setup();
    scrn_terminal.setup();
    scrn_ap_detail.setup();

    // スクリーンマネージャにスクリーンを追加
    // 最初に追加したスクリーンが最初に表示されるスクリーンになる
    scrn_manager.add_screen(SCREEN_ID_MAIN, &scrn_main);
    scrn_manager.add_screen(SCREEN_ID_SHUTDOWN, &scrn_shutdown);
    scrn_manager.add_screen(SCREEN_ID_TERMINAL, &scrn_terminal);
    scrn_manager.add_screen(SCREEN_ID_AP_DETAIL, &scrn_ap_detail);

    scrn_terminal.print("Terminal Screen Initialized.\n");
    scrn_terminal.print("Wi-Fi AP Finder Ready\n");

    // WiFiをStation mode (スキャン用) に設定
    WiFi.mode(WIFI_STA);
    WiFi.setAutoConnect(false);
    WiFi.setAutoReconnect(false);
    
    scrn_terminal.printf("Wi-Fi initialized for scanning\n");

    // debug用
    sys_status.debug1 = 0;
    sys_status.debug2 = 0;

    sys_status.imu_gz = 0.0f;
    sys_status.imu_gz_offset = 0.0f;
    sys_status.imu_gz_calib_count = 0;
    float gz_sum = 0.0f;
    // ジャイロZ軸のオフセット値をキャリブレーション
    for( int i=0; i<100; i++ ){
        update_imu_gz();
        gz_sum += sys_status.imu_gz;
        sys_status.imu_gz_calib_count++;
        delay(10);
    }
    if (sys_status.imu_gz_calib_count > 0) {
        sys_status.imu_gz_offset = gz_sum / sys_status.imu_gz_calib_count;
    }
}



void loop() 
{
    static uint32_t prev_sec = 0;
    uint32_t sec;

    M5.update();
    update_imu_gz();

    sec = millis() / 1000;
    if (sec != prev_sec){
        // 1秒ごとの処理をここに追加
        prev_sec = sec;
        sys_status.battery_level = M5.Power.getBatteryLevel();
        
        // WiFiスキャンステータスを表示
        int scan_status = WiFi.scanComplete();
        if (scan_status == WIFI_SCAN_RUNNING) {
            scrn_terminal.printf("%u - WiFi Scanning...\n", sec);
        }
        
        // デバッグ用の変数を表示
        // else {
        //    scrn_terminal.printf("%u - %d %d\n", sec, sys_status.debug1, sys_status.debug2);
        // }
    }

    // 現在表示されているスクリーンのループ処理
    scrn_manager.loop();

    // LVGLのタスクハンドラを呼び出す
    lv_task_handler();

    // 電源ボタンが押されたらシャットダウン画面へ
    if( M5.BtnPWR.wasClicked() ) {
        scrn_manager.change_screen(SCREEN_ID_SHUTDOWN, SCREEN_ANIM_NONE);
    }

    delay(10);
}

