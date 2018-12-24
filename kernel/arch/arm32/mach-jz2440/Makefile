extra-y	+= head.o
extra-y	+= init.o
extra-y	+= nand_ll.o

obj-y	+= lib/
obj-y	+= mm/
obj-y	+= drivers/
obj-y	+= $(filter-out $(extra-y), \
			$(patsubst $(srctree)/$(src)/%.c, %.o, $(wildcard $(srctree)/$(src)/*.c)) \
			$(patsubst $(srctree)/$(src)/%.S, %.o, $(wildcard $(srctree)/$(src)/*.S)))