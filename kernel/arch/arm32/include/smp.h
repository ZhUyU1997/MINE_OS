#ifndef __SMP_H__
#define __SMP_H__

#include <task.h>


#define smp_processor_id()	(current->cpu_id)

#endif
