TARGET			:= app

INCLUDEDIR 		:= $(CURDIR)/include
#WFLAGS			:= -Wall
WFLAGS			:= -w
CFLAGS 			:= -std=gnu99 $(WFLAGS) -O2 -fno-builtin -march=armv4t -mtune=arm920t -nostdlib -msoft-float -fsigned-char -fno-omit-frame-pointer
CPPFLAGS   		:= -I$(INCLUDEDIR) -nostdinc
LDFLAGS			:= -L$(shell dirname `$(CC) $(CFLAGS) $(CPPFLAGS) -print-libgcc-file-name`) -lgcc
LDFLAGS			+= -T$(strip $(TARGET)).lds


export CFLAGS CPPFLAGS LDFLAGS

obj-y += init/
obj-y += src/
obj-y += lib/

.PHONY : all dis clean distclean
all:
	@echo $(shell pwd):
	@make -s -C ./ -f $(TOPDIR)/Makefile.build
	@$(LD) -o system built-in.o $(LDFLAGS)
	@$(OBJCOPY) -O binary -S system $(TARGET).bin

dis:system
	@echo OBJDUMP $(TARGET).dis
	@$(OBJDUMP) -D -m arm system > $(TARGET).dis

clean:
	rm -f $(shell find -name "*.o")
	rm -f $(TARGET)

distclean:
	rm -f $(shell find -name "*.o")
	rm -f $(shell find -name "*.d")
	rm -f $(shell find -name "*.a")
	rm -f $(shell find -name "*.mac")
	rm -f $(TARGET) $(TARGET).dis $(TARGET).bin system