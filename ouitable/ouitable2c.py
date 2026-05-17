# !/usr/bin/env python3
# -*- coding: utf-8 -*-

# ouitabele2c.py
# ouit.txt を読み込んで、C言語の配列定義を出力するスクリプト．
# ベンダ名は最大31文字，MACアドレスのプレフィックスはuint32_tの配列で定義する．

import sys
import re

def main(argv):
    try:
        with open('oui.txt', 'r') as f:
            lines = f.readlines()
    except FileNotFoundError:
        print("Error: 'oui.txt' not found.")
        sys.exit(1)

    print('#include <stdint.h>')
    print()
    print('typedef struct {')
    print('    uint32_t prefix;')
    print('    const char *vendor;')
    print('} oui_entry_t;')
    print()
    print('const oui_entry_t oui_table[] = {')

    for line in lines:
        if line.startswith('#') or not line.strip():
            continue
        # "08-EA-44   (hex)		Extreme Networks Headquarters" のような行のみを処理する
        ptn = "^[0-9A-Fa-f]{2}-[0-9A-Fa-f]{2}-[0-9A-Fa-f]{2}\\s+\\(hex\\)\\s+(.*)$"
        if not re.match(ptn, line):
            continue
        parts = line.split()
        if len(parts) < 3:
            continue
        prefix_str = parts[0].replace('-', '')
        # ベンダ名はスペースを含む可能性があるため、parts[2]以降を結合する
        vendor_name = ' '.join(parts[2:])
        # 最大31文字に切り詰める
        vendor_name = vendor_name[:31]
        # non-ascii文字をスペースに置換する
        vendor_name = ''.join(c if ord(c) < 128 else ' ' for c in vendor_name)
        # '"'エスケープ
        vendor_name = vendor_name.replace('"', '\\"')
        prefix_int = int(prefix_str, 16)
        print(f'    {{0x{prefix_int:06X}, "{vendor_name}"}},')

    print('};')
    print()
    print('const size_t oui_table_size = sizeof(oui_table) / sizeof(oui_entry_t);')

if __name__ == '__main__':
    main(sys.argv)
