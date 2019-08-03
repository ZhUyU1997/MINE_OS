PHONY		:=
all:

# Do not:
# o  use make's built-in rules and variables
#    (this increases performance and avoids hard-to-debug behaviour);
# o  print "Entering directory ...";
MAKEFLAGS	+= -rR -s --no-print-directory

BUILD_SRC	:=
BUILD_OBJ	:=

MKDIR		:=	mkdir -p
CP			:=	cp -af
RM			:=	rm -rf
CD			:=	cd
MV			:=	mv
FIND		:=	find

export RM CP CD MV FIND MKDIR

ifeq ($(BUILD_SRC),)

ifeq ("$(origin O)", "command line")
BUILD_OBJ	:= $(O)
endif

ifneq ($(BUILD_OBJ),)
PHONY	+=	all $(MAKECMDGOALS) clean

__dummy		:= $(shell $(MKDIR) $(BUILD_OBJ))

ifneq ($(__dummy),)
$(error failed to create $(BUILD_OBJ))
endif

$(filter-out all,$(MAKECMDGOALS)) all: sub-mk

sub-mk:
	@$(MAKE) -C $(BUILD_OBJ) BUILD_SRC=$(CURDIR) -f $(CURDIR)/Makefile $(MAKECMDGOALS)

clean: sub-mk
	@echo [RM] $(BUILD_OBJ)
	@$(RM) $(BUILD_OBJ)

skip-makefile := 1
endif #ifneq ($(BUILD_OBJ),)
endif #ifeq ($(BUILD_SRC),)


ifeq ($(skip-makefile),)

CROSS_COMPILE	?=arm-linux-gnueabihf-
PLATFORM		?=

srctree		:= $(if $(BUILD_SRC),$(BUILD_SRC),$(CURDIR))
objtree		:= $(CURDIR)
src			:= .
obj			:= .

VPATH		:= $(srctree)

export srctree objtree VPATH

# Get platform information about ARCH and MACH from PLATFORM variable.
ifeq ($(words $(subst -, , $(PLATFORM))), 2)
ARCH		:= $(word 1, $(subst -, , $(PLATFORM)))
MACH		:= mach-$(word 2, $(subst -, , $(PLATFORM)))
else
ARCH		:= arm32
MACH		:= mach-x4418
endif

ifeq ($(wildcard $(srctree)/arch/$(ARCH)/$(MACH)),)
$(error not found the $(ARCH) or $(MACH))
endif


# System environment variable.
ifeq ($(OS),Windows_NT)
	HOSTOS		:= windows
else
	ifeq ($(shell uname),Darwin)
		HOSTOS		:= macos
	else
		ifeq ($(shell uname),Linux)
			HOSTOS		:= linux
		else
			HOSTOS		:= unix-like
		endif
	endif
endif

ifeq ($(HOSTOS),windows)
	SUFFIX	:= .exe
else
	SUFFIX	:=
endif
# Load default variables.
CFLAGS		:=
ASFLAGS		:=
LDFLAGS		:=
ARCH_CFLAGS	:=
MCFLAGS		:=
MLDFLAGS	:=
DEFINES		:=

LIBDIRS		:=
LIBS 		:=
INCDIRS		:=
SRCDIRS		:=

# Make variables (CC, etc...)
AS			:=	$(CROSS_COMPILE)gcc -x assembler-with-cpp
CC			:=	$(CROSS_COMPILE)gcc
CPP			:=	$(CROSS_COMPILE)gcc -E
CXX			:=	$(CROSS_COMPILE)g++
LD			:=	$(CROSS_COMPILE)ld
AR			:=	$(CROSS_COMPILE)ar
OC			:=	$(CROSS_COMPILE)objcopy
OD			:=	$(CROSS_COMPILE)objdump
NM			:=	nm

HOSTCC		:=	gcc

# Override default variables.
sinclude $(srctree)/arch/$(ARCH)/$(MACH)/Makefile.mach
sinclude $(srctree)/arch/$(ARCH)/Makefile.arch

# We need some generic definitions (do not try to remake the file).
include $(srctree)/scripts/Makefile.include

# Add necessary directory for INCDIRS and SRCDIRS.
SRCDIRS		+=	arch/$(ARCH) src lib
INCDIRS		+=	include	drivers					\
				arch/$(ARCH)/include			\
				arch/$(ARCH)/$(MACH)/include

HEAD_FILES	:=	arch/$(ARCH)/$(MACH)/head.S

sinclude $(srctree)/arch/$(ARCH)/$(MACH)/Makefile.head

HEAD_FILES	:=	$(foreach f,$(HEAD_FILES),$(wildcard $(srctree)/$(f)))
HEAD_OBJS	+=	$(patsubst $(srctree)/%.S, %.o, $(filter %.S, $(HEAD_FILES))) \
				$(patsubst $(srctree)/%.c, %.o, $(filter %.c, $(HEAD_FILES)))

X_ASFLAGS	:=	$(ARCH_CFLAGS) $(MCFLAGS) $(MASFLAGS) $(ASFLAGS)
X_CFLAGS	:=	$(ARCH_CFLAGS) $(MCFLAGS) $(CFLAGS)

X_LDFLAGS	:=	$(MLDFLAGS) $(LDFLAGS)
X_LIBS		:=	$(LIBS)

X_NAME		:=	init

X_INCDIRS	:=	$(patsubst %, -I %, \
				$(foreach d,$(INCDIRS),$(wildcard $(srctree)/$(d))))
X_CPPFLAGS	:=	$(DEFINES) $(X_INCDIRS) -include include/config/autoconf.h

X_LIBDIRS	:=	$(LIBDIRS)
X_LDFLAGS	+=	$(X_LIBDIRS)

X_OBJS		:=	$(patsubst $(srctree)/%, %/built-in.o, \
				$(foreach d,$(SRCDIRS),$(wildcard $(srctree)/$(d)))) \
				$(HEAD_OBJS)

export BUILD_OBJ BUILD_SRC HEAD_OBJS HOSTOS
export ARCH MACH
export AS AR CC LD CPP CXX
export X_ASFLAGS X_INCDIRS X_CFLAGS X_CPPFLAGS DEFINES

include $(srctree)/scripts/Makefile.conf

PHONY	+=	all clean xbegin xend xclean conf fixdep dis dnw $(SRCDIRS)


# MINE rules
all: xend

xend : $(X_NAME).bin

$(SRCDIRS):	xbegin

xbegin: scripts/fixdep$(SUFFIX) conf

scripts/fixdep$(SUFFIX): $(srctree)/scripts/fixdep.c
	@$(MKDIR) scripts
	@echo [HOSTCC] scripts/fixdep.c
ifeq ($(strip $(HOSTOS)),windows)
	@$(HOSTCC) -o $@ $< -lwsock32
else
	@$(HOSTCC) -o $@ $<
endif

$(X_NAME).bin: $(X_OBJS)
	@echo [LD] system
	@$(LD) $(X_LDFLAGS) -o system $(X_OBJS) $(X_LIBS)
	@echo [OC] $(X_NAME).bin
	@$(OC) -O binary -S system $@

$(X_OBJS): $(SRCDIRS) ;

$(SRCDIRS):
	@$(MAKE) $(build)=$@

dis: system
	@echo [OD] $(X_NAME).dis
	@$(OD) -D -m arm system > $(X_NAME).dis

dnw: $(X_NAME).bin
	dnw $(X_NAME).bin

clean:
	@echo [RM] system
	@$(RM) system 
	@echo [RM] fixdep
	@$(RM) scripts/fixdep$(SUFFIX)
	@echo [RM] *.[osd]
	@$(RM) $(shell find -name "*.[osd]")
	@echo [RM] *.[osSc]~
	@$(RM) $(shell find -name "*.[osSc]~")
	@echo [RM] .*.cmd
	@$(RM) $(shell find -name ".*.cmd")
	@echo [RM] include/config
	@$(RM) include/config
	@echo [RM] $(X_NAME).*
	@$(RM) $(X_NAME).*

endif

PHONY += FORCE

FORCE:

# Declare the contents of the .PHONY variable as phony.  We keep that
# information in a variable so we can use it in if_changed and friends.
.PHONY: $(PHONY)