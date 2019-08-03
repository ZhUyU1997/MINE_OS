extra-y	+= head.o

obj-y	+= lib/
obj-y	+= $(filter-out $(extra-y), \
			$(patsubst $(srctree)/$(src)/%.c, %.o, $(wildcard $(srctree)/$(src)/*.c)) \
			$(patsubst $(srctree)/$(src)/%.S, %.o, $(wildcard $(srctree)/$(src)/*.S)))
