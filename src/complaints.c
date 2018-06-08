#include <stdio.h>
int raise(int signum) {
	printf("raise\n");
	return 0;
}
void __aeabi_unwind_cpp_pr0(void) {
	printf("__aeabi_unwind_cpp_pr0\n");
};
void __aeabi_unwind_cpp_pr1(void) {
	printf("__aeabi_unwind_cpp_pr1\n");
};
