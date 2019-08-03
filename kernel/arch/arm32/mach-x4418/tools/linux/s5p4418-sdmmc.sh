#!/bin/sh
#
# s5p4418 irom sd/mmc boot fusing tool.
# Author: Jianjun Jiang <8192542@qq.com>


# display usage message
USAGE()
{
  echo Usage: $(basename "$0") '<device> <bootloader>'
  echo '       device     = disk device name for SD card.'
  echo '       bootloader = /path/to/xbootpak.bin'
  echo 'e.g. '$(basename "$0")' /dev/sdb xbootpak.bin'
}

[ `id -u` == 0 ] || { echo "you must be root user"; exit 1; }
[ -z "$1" -o -z "$2" ] && { USAGE; exit 1; }

dev="$1"
xboot="$2"

# validate parameters
[ -b "${dev}" ] || { echo "${dev} is not a valid block device"; exit 1; }
[ X"${dev}" = X"${dev%%[0-9]}" ] || { echo "${dev} is a partition, please use device, perhaps ${dev%%[0-9]}"; exit 1; }
[ -f ${xboot} ] || { echo "${xboot} is not a bootloader binary file."; exit 1; }

# copy the full bootloader image to block device
dd if="${xboot}" of="${dev}" bs=512 seek=1 conv=sync

sync;
sync;
sync;

echo "^_^ The image is fused successfully"

