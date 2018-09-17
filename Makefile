CROSS_COMPILE	= arm-none-linux-gnueabi-

#
# System environment variable.
#
ifneq (,$(findstring Linux, $(shell uname -s)))
HOSTOS		:= linux
else
HOSTOS		:= windows
endif

#CURDIR为内置变量
TOPDIR			:= $(CURDIR)


AS				= $(CROSS_COMPILE)as
LD				= $(CROSS_COMPILE)ld
CC				= $(CROSS_COMPILE)gcc
CPP				= $(CC) -E
AR				= $(CROSS_COMPILE)ar
NM				= $(CROSS_COMPILE)nm

STRIP			= $(CROSS_COMPILE)strip
OBJCOPY			= $(CROSS_COMPILE)objcopy
OBJDUMP			= $(CROSS_COMPILE)objdump

RM				= rm

export AS LD CC CPP AR NM
export STRIP OBJCOPY OBJDUMP
export RM
export HOSTOS TOPDIR

.PHONY : all kernel user dis dnw clean distclean
all:kernel

kernel:
	@make -s -C ./kernel all

user:
	@make -s -C ./user all

dis:
	@make -s -C ./kernel dis

dnw:
	@make -s -C ./kernel dnw

clean:
	@make -s -C ./kernel clean
	@make -s -C ./user clean

distclean:
	@make -s -C ./kernel distclean
	@make -s -C ./user distclean