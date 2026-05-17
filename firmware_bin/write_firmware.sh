#!/bin/sh

# M5Stack Core2 ファームウエア書き込みスクリプト．Linux用．
# esotoolをインストールしておく必要があります．
# 例えば，Ubuntuの場合は以下のコマンドでインストールできます．
# sudo apt install esptool

SERIALPORT=/dev/ttyACM0

esptool --chip esp32 --port $SERIALPORT --baud 921600 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size detect 0x1000 bootloader.bin 0x8000 partitions.bin 0x10000 firmware.bin

