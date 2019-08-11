#!/bin/sh
#

# The qemu's root directory.
export QEMU_DIR=$(cd `dirname $0` ; pwd)

cd ${QEMU_DIR}
if [ ! -e ${QEMU_DIR}/sdcard/sdcard.img ]; then
	mkdir -p ${QEMU_DIR}/sdcard;
	unzip ${QEMU_DIR}/sdcard.zip -d ${QEMU_DIR}/sdcard > /dev/null;
fi

# Run qemu
setsid bash -c "deepin-terminal -x qemu-system-arm -M realview-pb-a8 -m 256M \
-name \"ARM RealView Platform Baseboard for Cortex-A8\" \
-S -gdb tcp::10001,ipv4 \
-show-cursor -rtc base=localtime -serial stdio \
-sd sdcard/sdcard.img \
-net nic,macaddr=88:88:88:11:22:33,model=lan9118 -net user \
-kernel ../../../kernel/system &"
