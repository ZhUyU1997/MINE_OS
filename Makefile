.PHONY : all kernel user dis dnw clean distclean

all:kernel

kernel:
	@$(MAKE) -s -C ./kernel all
user:
	@$(MAKE) -s -C ./user all
dis:
	@$(MAKE) -s -C ./kernel dis
dnw:
	@$(MAKE) -s -C ./kernel dnw
clean:
	@$(MAKE) -s -C ./kernel clean
	@$(MAKE) -s -C ./user clean