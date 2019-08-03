#include <stdio.h>
#include <assert.h>

int raise(int signum) {
	printf("raise\n");
	assert(0);
	return 0;
}
void __aeabi_unwind_cpp_pr0(void) {
	printf("__aeabi_unwind_cpp_pr0\n");
};
void __aeabi_unwind_cpp_pr1(void) {
	printf("__aeabi_unwind_cpp_pr1\n");
};

void __div0(void)
{
	printf("Attempting division by 0!");
}
