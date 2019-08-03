extra-y	+= head.o

obj-y	+= driver/
obj-y	+= $(filter-out $(extra-y), \
			$(patsubst $(srctree)/$(src)/%.c, %.o, $(wildcard $(srctree)/$(src)/*.c)) \
			$(patsubst $(srctree)/$(src)/%.S, %.o, $(wildcard $(srctree)/$(src)/*.S)))

$(obj)/dtree.o : $(src)/x4418.json
