#include <assert.h>
#include <stdio.h>
void __Assert(char *msg){
	printf("Assertion failed: %s\n",msg);
	disable_irq();
	while(1);
}