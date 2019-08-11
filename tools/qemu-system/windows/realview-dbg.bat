@echo off
@set SDL_STDIO_REDIRECT=0
@set QEMU_DIR=%~dp0

cd %QEMU_DIR%

chcp 65001

if exist "sdcard\sdcard.img" goto RunQemu
    mkdir "sdcard\"
    unzip.exe sdcard.zip -d "sdcard"

:RunQemu
start qemu-system-arm.exe -M realview-pb-a8 -m 256M ^
-name "ARM RealView Platform Baseboard for Cortex-A8" ^
-S -gdb tcp::10001,ipv4 ^
-show-cursor -rtc base=localtime -serial stdio ^
-sd "sdcard\sdcard.img" ^
-net nic,macaddr=88:88:88:11:22:33,model=lan9118 -net user ^
-kernel ..\..\..\kernel\system
