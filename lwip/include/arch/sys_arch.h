#ifndef __SYS_ARCH_H__
#define __SYS_ARCH_H__

#include "cpu.h"
#include "ucos_ii.h"

/* NULL ¶¨Òå */
#ifndef NULL
#define NULL 0
#endif

/* HANDLE is used for sys_sem_t but we won't include windows.h */
typedef OS_EVENT* sys_sem_t;
#define SYS_SEM_NULL NULL
#define sys_sem_valid(sema) ((*sema) != NULL)
#define sys_sem_set_invalid(sema) ((*sema) = NULL)

/* let sys.h use binary semaphores for mutexes */
#define LWIP_COMPAT_MUTEX 1

#ifndef MAX_QUEUE_ENTRIES
#define MAX_QUEUE_ENTRIES 100
#endif
struct lwip_mbox {
  sys_sem_t sem;
  sys_sem_t mutex;
  void* q_mem[MAX_QUEUE_ENTRIES];
  u32_t head, tail;
  u32_t msg_num; 
};
typedef struct lwip_mbox sys_mbox_t;
#define SYS_MBOX_NULL NULL
#define sys_mbox_valid(mbox) ((mbox != NULL) && ((mbox)->sem != NULL))
#define sys_mbox_set_invalid(mbox) ((mbox)->sem = NULL)

/* DWORD (thread id) is used for sys_thread_t but we won't include windows.h */
typedef INT8U sys_thread_t;



#endif /* __SYS_ARCH_H__ */
