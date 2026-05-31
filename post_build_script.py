# ビルド成功後にバイナリを別ディレクトリにコピーするスクリプト

Import("env")
import os
import shutil

def copy_bins(source, target, env):
    build_dir = env.subst("$BUILD_DIR")

    # コピーするファイルのリスト
    bin_files = [
        "bootloader.bin",
        "firmware.bin",
        "partitions.bin",
    ]

    # コピー先ディレクトリ
    dest_dir = "firmware_bin"
    os.makedirs(dest_dir, exist_ok=True)

    copied = []

    for name in bin_files:
        src = os.path.join(build_dir, name)
        if os.path.exists(src):
            shutil.copy(src, dest_dir)
            copied.append(name)
        else:
            print(f"[POST-BUILD] Warning: {name} not found")

    if copied:
        print("[POST-BUILD] Copied:", ", ".join(copied))
    else:
        print("[POST-BUILD] No bin files copied")

# ビルド成功後のみ実行
action = env.AddPostAction("buildprog", copy_bins)
env.AlwaysBuild(action)
