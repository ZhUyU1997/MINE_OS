#include <core/machine.h>
#include <assert.h>

int raise(int signum) {
	LOG("raise");
	assert(0);
	return 0;
}
void __aeabi_unwind_cpp_pr0(void) {
	LOG("__aeabi_unwind_cpp_pr0");
};
void __aeabi_unwind_cpp_pr1(void) {
	LOG("__aeabi_unwind_cpp_pr1");
};

void __div0(void)
{
	LOG("Attempting division by 0!");
}
