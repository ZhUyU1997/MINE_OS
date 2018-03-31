#include <stdio.h>
void __aeabi_unwind_cpp_pr0() {
	printf("__aeabi_unwind_cpp_pr0\n");
}
int raise(int signum) {
	printf("raise\n");
	return 0;
}
