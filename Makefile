.PHONY : all kernel user dis clean

all:kernel

kernel:
	@$(MAKE) -s -C ./kernel all
user:
	@$(MAKE) -s -C ./user all
dis:
	@$(MAKE) -s -C ./kernel dis
clean:
	@$(MAKE) -s -C ./kernel clean
	@$(MAKE) -s -C ./user clean
