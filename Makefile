CROSS_COMPILE	= arm-none-linux-gnueabi-

#
# System environment variable.
#
ifneq (,$(findstring Linux, $(shell uname -s)))
HOSTOS		:= linux
else
HOSTOS		:= windows
endif


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

ifeq ($(strip HOSTOS),linux)
TOPDIR			:= $(shell pwd)
else
TOPDIR			:= $(subst /c,c:,$(shell pwd))
endif


INCLUDEDIR 		:= $(TOPDIR)/include
#WFLAGS			:= -Wall
WFLAGS			:= -w
CFLAGS 			:= -std=gnu99 $(WFLAGS) -O2 -fno-builtin -march=armv4t -mtune=arm920t -nostdlib -msoft-float -fsigned-char -fno-omit-frame-pointer
CPPFLAGS   		:= -I$(INCLUDEDIR) -I$(TOPDIR)/drivers -I$(TOPDIR)/fs/Fatfs_f8a -nostdinc
LDFLAGS			:= -L$(shell dirname `$(CC) $(CFLAGS) $(CPPFLAGS) -print-libgcc-file-name`) -lgcc
LDFLAGS			+= -Tmine.lds

export CFLAGS CPPFLAGS LDFLAGS
export TOPDIR

TARGET := mine

obj-y += init/
obj-y += drivers/
obj-y += src/
obj-y += lib/
obj-y += mm/
obj-y += fs/
obj-y += sound/

.PHONY : all dis download clean distclean
all:
	@echo $(shell pwd):
	@make -s -C ./ -f $(TOPDIR)/Makefile.build
	@$(LD) -o system_temp built-in.o $(LDFLAGS)
	@gcc -o kallsyms $(TOPDIR)/scripts/kallsyms.c
	@nm -n system_temp | ./kallsyms > kallsyms.S
	@$(CC) $(CFLAGS) $(CPPFLAGS) -c -o kallsyms.o kallsyms.S
	@echo LD system
	@$(LD) -o system built-in.o kallsyms.o $(LDFLAGS)
	@echo OBJCOPY $(TARGET).bin
	@$(OBJCOPY) -O binary -S system $(TARGET).bin

dis:system
	@echo OBJDUMP $(TARGET).dis
	@$(OBJDUMP) -D -m arm system > $(TARGET).dis

dnw:$(TARGET).bin
	dnw $(TARGET).bin

clean:
	rm -f $(shell find -name "*.o")
	rm -f $(TARGET)

distclean:
	rm -f $(shell find -name "*.o")
	rm -f $(shell find -name "*.d")
	rm -f $(shell find -name "*.a")
	rm -f $(shell find -name "*.mac")
	rm -f $(TARGET) $(TARGET).dis $(TARGET).bin system system_temp kallsyms.S
ifeq ($(strip HOSTOS),linux)
	rm -f kallsyms
else
	rm -f kallsyms.exe
endif