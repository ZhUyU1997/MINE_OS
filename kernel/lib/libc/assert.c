#include <assert.h>
#include <stdio.h>
#include <irqflags.h>

void __Assert(char *msg){
	printf("Assertion failed: %s\n",msg);
	local_irq_disable();
	while(1);
}
