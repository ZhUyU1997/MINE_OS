obj-y += lv_core/
obj-y += lv_hal/
obj-y += lv_objx/
obj-y += lv_fonts/
obj-y += lv_misc/
obj-y += lv_themes/
obj-y += lv_draw/

obj-y += port/

SUBDIR_CPPFLAGS	+= $(patsubst %/, -I $(src)/%, $(obj-y))
SUBDIR_CPPFLAGS	+= -I$(src)
SUBDIR_CPPFLAGS	+= -DLV_CONF_INCLUDE_SIMPLE
