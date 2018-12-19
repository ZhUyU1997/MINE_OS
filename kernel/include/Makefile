PHONY	+= conf

conf : $(obj)/config/auto.conf $(obj)/config/autoconf.h

$(obj)/config/autoconf.h : $(srctree)/$(src)/kconfigs.h \
		$(wildcard $(srctree)/arch/$(ARCH)/$(MACH)/include/configs.h)
	$(if $(wildcard $(obj)/config),:,@echo [MKDIR] $(obj)/config && $(MKDIR) $(obj)/config)
	$(if $(wildcard $(obj)/config/autoconf.h),:,@touch $(obj)/config/autoconf.h)
	@$(CC) -E -P -dM $(DEFINES) $(X_INCDIRS) $(srctree)/$(src)/kconfigs.h \
	| sed -n -e "/\#define\s\+CONFIG_[[:alnum:]_]*/p" \
	| sed -n -e "s/\s*$$//p" \
	> $(obj)/config/autoconf.temp
	@$(CD) $(obj)/config && grep -vxf autoconf.temp autoconf.h \
	| sed -n -e "s/\#define\s\+CONFIG_\([[:alnum:]_]*\).*/\L\1\E.h/p" \
	| xargs $(RM)
	@$(CD) $(obj)/config && grep -vxf autoconf.h autoconf.temp \
	| sed -n -e "s/\#define\s\+CONFIG_\([[:alnum:]_]*\).*/\L\1\E.h/p" \
	| xargs touch autoconf.h
	@$(RM) $(obj)/config/autoconf.h
	@echo [AUTOCONF]
	@$(MV) $(obj)/config/autoconf.temp $(obj)/config/autoconf.h

$(obj)/config/auto.conf : $(obj)/config/autoconf.h
	@$(RM) $(obj)/config/auto.conf
	@$(CD) $(obj)/config && cat autoconf.h \
	| sed -n -e "s/\#define\s\+\(CONFIG_[[:alnum:]_]*\)[[:space:]]/\1=/p" \
	>> auto.conf
	@$(CD) $(obj)/config && cat autoconf.h \
	| sed -n -e "s/\#define\s\+\(CONFIG_[[:alnum:]_]*\)$$/\1=y/p" \
	>> auto.conf
