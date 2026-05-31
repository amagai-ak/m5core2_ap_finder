/**
 * @file system_status.h
 * @author amagai
 * @brief システムステータス管理用ヘッダファイル
 * @version 0.1
 * @date 2026-05-17
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#ifndef SYSTEM_STATUS_H
#define SYSTEM_STATUS_H

typedef struct {
    uint8_t bssid[6];
    char ssid[32];
    int channel;
    int rssi;
    int nfloor; // ノイズフロア: ESP32では実際には固定値が返されるのみ．
    uint32_t rssi_timestamp; // rssiが更新された時刻（millis()）
    uint32_t framecount_management;
    uint32_t framecount_control;
    uint32_t framecount_data;
} ap_info_t;


typedef struct {
    int battery_level;  // バッテリー残量 (%)
    ap_info_t target_ap; // 観測対象AP情報
    float imu_gz; // ジャイロZ軸の値
    float imu_gz_offset; // ジャイロZ軸のオフセット値
    int imu_gz_calib_count; // ジャイロZ軸のキャリブレーションに使用したサンプル数
    uint32_t imu_gz_timestamp; // ジャイロZ軸の値が更新された時刻（millis()）
    // デバッグ用の変数
    int debug1;
    int debug2;
} system_status_t;

extern system_status_t sys_status;

#endif // SYSTEM_STATUS_H
