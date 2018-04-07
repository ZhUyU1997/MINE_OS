#include <stdio.h>
int raise(int signum) {
	printf("raise\n");
	return 0;
}
