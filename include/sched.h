/* 条件编译 */
#ifndef __TASK_H__
#define __TASK_H__
#include <sys/param.h>
#include <sys/types.h>

#define TASK_UNALLOCATE			-1	// PCB未分配
#define TASK_RUNNING			0	// 进程正在进行或已经准备就绪
#define TASK_INTERRUPTIBLE		1	// 进程处于可中断等待状态
#define TASK_UNINTERRUPTIBLE	2	// 进程处于不可中断等待状态
#define TASK_ZOMBIE				3	// 进程处于僵死状态，未用到
#define TASK_STOPPED			4	// 进程已经停止
#define TASK_SLEEPING			5	// 进程进入睡眠状态

#define PID_OFT				0
#define	STATE_OFT			4
#define	COUNT_OFT			8
#define	PRIORITY_OFT		16
#define	CONTENT_OFT			20

struct cpu_context_save {
	__u32 cpsr;
	__u32 sp;
	__u32 lr;
	__u32 r0;
	__u32 r1;
	__u32 r2;
	__u32 r3;
	__u32 r4;
	__u32 r5;
	__u32 r6;
	__u32 r7;
	__u32 r8;
	__u32 r9;
	__u32 r10;
	__u32 r11;
	__u32 r12;
	__u32 pc;
	
};
struct inode;
typedef struct task_struct
{
	long pid;						// 进程ID
	long state;						// 进程状态
	long count;						// 进程时间片数
	long timer;						// 进程休眠时间
	unsigned long priority;			// 进程优先级
	__u32 stack;
	struct cpu_context_save context;		// 进程执行现场保存区（寄存器的值）
	struct file *files[NR_OPEN];
	struct inode *pwd;
	 uid_t uid;
	 gid_t gid;
} PCB;
struct mm_struct {
    unsigned long total_vm, locked_vm, shared_vm, exec_vm;
    //total_vm 进程地址空间的大小(页数）
    //locked_vm 锁住而不能换出的页的个数
    //shared_vm 共享文件内存映射中的页数
    unsigned long stack_vm, reserved_vm, def_flags, nr_ptes;
    //stack_vm 用户堆栈中的页数
    //reserved_vm 在保留区中的页数或者在特殊线性区中的页数
    //def_flags 线性区默认的访问标志
    //nr_ptes 进程的页表数
    unsigned long start_code, end_code, start_data, end_data;
    //start_code 可执行代码的起始地址
    //end_code 可执行代码的最后地址
    //start_data已初始化数据的起始地址
    // end_data已初始化数据的最后地址
    unsigned long start_brk, brk, start_stack;
    //start_stack堆的起始位置
    //brk堆的当前的最后地址
    //用户堆栈的起始地址
    unsigned long arg_start, arg_end, env_start, env_end;
    //arg_start 命令行参数的起始地址
    //arg_end命令行参数的起始地址
    //env_start环境变量的起始地址
    //env_end环境变量的最后地址
};
// 进程队列数组
extern PCB task[NR_TASK];
// 当前进程结构体指针
extern PCB *__current_task;
// 正在运行进程个数
extern long runningCount;
// 进程调度
extern void schedule(void);
// 杀死进程
extern void kthread_kill(int pid);
// 进程调度初始化
extern void sched_init(void);
// 创建进程
extern int kthread_create(unsigned long pc, void *data, long priority);
#define CURRENT_TASK() (__current_task)
#endif
