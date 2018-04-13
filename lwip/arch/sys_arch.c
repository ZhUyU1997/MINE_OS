#include <ucos_ii.h>
unsigned long sys_now(void){
	return ((int)OSTimeGet());
}