#include <task.h>
long preempt_count(){
	return current->preempt_count;
}

#define preempt_count()	(current->preempt_count)

void add_preempt_count(int val){
	preempt_count() += (val);
}
void sub_preempt_count(int val){
	preempt_count() -= (val);
}


