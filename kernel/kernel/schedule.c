/***************************************************
*		版权声明
*
*	本操作系统名为：MINE
*	该操作系统未经授权不得以盈利或非盈利为目的进行开发，
*	只允许个人学习以及公开交流使用
*
*	代码最终所有权及解释权归田宇所有；
*
*	本模块作者：	田宇
*	EMail:		345538255@qq.com
*
*
***************************************************/

#include <schedule.h>
#include <task.h>
#include <lib.h>
#include <printk.h>
#include <smp.h>
#include <compiler.h>
#include <clocksource/clocksource.h>
#include <malloc.h>

struct scheduler_t __sched[NR_CPUS];

static inline struct scheduler_t * scheduler_self(void)
{
	return &__sched[smp_processor_id()];
}

static inline struct task_t * task_self(void)
{
	return __sched[smp_processor_id()].running;
}

static const int nice_to_weight[40] = {
	/* -20 */     88761,     71755,     56483,     46273,     36291,
	/* -15 */     29154,     23254,     18705,     14949,     11916,
	/* -10 */      9548,      7620,      6100,      4904,      3906,
	/*  -5 */      3121,      2501,      1991,      1586,      1277,
	/*   0 */      1024,       820,       655,       526,       423,
	/*   5 */       335,       272,       215,       172,       137,
	/*  10 */       110,        87,        70,        56,        45,
	/*  15 */        36,        29,        23,        18,        15,
};

static const uint32_t nice_to_wmult[40] = {
	/* -20 */     48388,     59856,     76040,     92818,    118348,
	/* -15 */    147320,    184698,    229616,    287308,    360437,
	/* -10 */    449829,    563644,    704093,    875809,   1099582,
	/*  -5 */   1376151,   1717300,   2157191,   2708050,   3363326,
	/*   0 */   4194304,   5237765,   6557202,   8165337,  10153587,
	/*   5 */  12820798,  15790321,  19976592,  24970740,  31350126,
	/*  10 */  39045157,  49367440,  61356676,  76695844,  95443717,
	/*  15 */ 119304647, 148102320, 186737708, 238609294, 286331153,
};

static inline uint64_t mul_u32_u32(uint32_t a, uint32_t b)
{
	return (uint64_t)a * b;
}

static inline uint64_t mul_u64_u32_shr(uint64_t a, uint32_t mul, unsigned int shift)
{
	uint32_t ah, al;
	uint64_t ret;

	al = a;
	ah = a >> 32;

	ret = mul_u32_u32(al, mul) >> shift;
	if(ah)
		ret += mul_u32_u32(ah, mul) << (32 - shift);
	return ret;
}

static inline uint64_t calc_delta(struct task_t * task, uint64_t delta)
{
	uint64_t fact = 1024;
	int shift = 32;

	if(unlikely(fact >> 32))
	{
		while(fact >> 32)
		{
			fact >>= 1;
			shift--;
		}
	}
	fact = (uint64_t)(uint32_t)fact * task->inv_weight;

	while(fact >> 32)
	{
		fact >>= 1;
		shift--;
	}
	return mul_u64_u32_shr(delta, fact, shift);
}

static inline uint64_t calc_delta_fair(struct task_t * task, uint64_t delta)
{
	if(unlikely(task->weight != 1024))
		delta = calc_delta(task, delta);
	return delta;
}

static inline struct task_t * scheduler_next_ready_task(struct scheduler_t * sched)
{
	struct rb_node * leftmost = rb_first_cached(&sched->ready);

	if(!leftmost)
		return NULL;
	return rb_entry(leftmost, struct task_t, node);
}

static inline void scheduler_enqueue_task(struct scheduler_t * sched, struct task_t * task)
{
	struct rb_node ** link = &sched->ready.rb_root.rb_node;
	struct rb_node * parent = NULL;
	struct task_t * next, * entry;
	int leftmost = 1;

	while(*link)
	{
		parent = *link;
		entry = rb_entry(parent, struct task_t, node);
		if((int64_t)(task->vtime - entry->vtime) < 0)
		{
			link = &parent->rb_left;
		}
		else
		{
			link = &parent->rb_right;
			leftmost = 0;
		}
	}

	rb_link_node(&task->node, parent, link);
	rb_insert_color_cached(&task->node, &sched->ready, leftmost);
	next = scheduler_next_ready_task(sched);
	if(likely(next))
		sched->min_vtime = next->vtime;
	else if(sched->running)
		sched->min_vtime = sched->running->vtime;
	else
		sched->min_vtime = 0;
}

static inline void scheduler_dequeue_task(struct scheduler_t * sched, struct task_t * task)
{
	struct task_t * next;

	rb_erase_cached(&task->node, &sched->ready);
	next = scheduler_next_ready_task(sched);
	if(likely(next))
		sched->min_vtime = next->vtime;
	else if(sched->running)
		sched->min_vtime = sched->running->vtime;
	else
		sched->min_vtime = 0;
}

static inline void scheduler_switch_task(struct scheduler_t * sched, struct task_t * task)
{
	struct task_t * running = sched->running;
	sched->running = task;
	//printf("[");
	//color_printk(GREEN,BLACK,"PC=%X, SP=%X, current PGD=%X, tsk PGD=%X", tsk->cpu_context.pc, tsk->cpu_context.sp, current->mm->pgd, tsk->mm->pgd);
	switch_mm(running, task);
	switch_to(running, task);
	//printf("]\n");
}

static inline struct scheduler_t * scheduler_load_balance_choice(void)
{
	struct scheduler_t * sched;
	uint64_t weight = ~0ULL;
	int i;

	for(i = 0; i < NR_CPUS; i++)
	{
		if(__sched[i].weight < weight)
		{
			sched = &__sched[i];
			weight = __sched[i].weight;
		}
	}
	return sched;
}

struct task_t * task_create(struct scheduler_t * sched, const char * name, size_t stksz, int nice)
{
	struct task_t * task;
	void * stack;

	if(!sched)
		sched = scheduler_load_balance_choice();

	if(stksz <= 0)
		stksz = CONFIG_TASK_SIZE;

	if(nice < -20)
		nice = -20;
	else if(nice > 19)
		nice = 19;

	stack = kzalloc(stksz);
	if(!stack)
	{
		free(task);
		return NULL;
	}

	task = (struct task_t *)stack;

	RB_CLEAR_NODE(&task->node);
	spin_lock(&sched->lock);
	list_add_tail(&task->list, &sched->suspend);
	init_list_head(&task->child_list);
	init_list_head(&task->child_node);
	sched->weight += nice_to_weight[nice + 20];
	spin_unlock(&sched->lock);

	task->name = strdup(name);
	task->status = TASK_STATUS_SUSPEND;
	task->start = ktime_to_ns(ktime_get());
	task->time = 0;
	task->vtime = 0;
	task->sched = sched;
	task->nice = nice;
	task->weight = nice_to_weight[nice + 20];
	task->inv_weight = nice_to_wmult[nice + 20];

	return task;
}

void task_destroy(struct task_t * task)
{
	if(task)
	{
		spin_lock(&task->sched->lock);
		task->sched->weight -= nice_to_weight[task->nice + 20];
		spin_unlock(&task->sched->lock);

		if(task->name)
			free(task->name);
		free(task);
	}
}

void task_zombie(struct task_t * task)
{
	struct task_t * next;
	uint64_t now, detla;

	if(task && task->status == TASK_STATUS_RUNNING)
	{
		now = ktime_to_ns(ktime_get());
		detla = now - task->start;

		task->time += detla;
		task->vtime += calc_delta_fair(task, detla);
		task->status = TASK_STATUS_ZOMBIE;
		spin_lock(&task->sched->lock);
		list_add_tail(&task->list, &task->sched->suspend);
		task->sched->weight -= nice_to_weight[task->nice + 20];
		spin_unlock(&task->sched->lock);

		next = scheduler_next_ready_task(task->sched);
		if(next)
		{
			scheduler_dequeue_task(task->sched, next);
			next->status = TASK_STATUS_RUNNING;
			next->start = now;
			scheduler_switch_task(task->sched, next);
		}
	}
}

void task_renice(struct task_t * task, int nice)
{
	if(nice < -20)
		nice = -20;
	else if(nice > 19)
		nice = 19;

	if(task->nice != nice)
	{
		spin_lock(&task->sched->lock);
		task->sched->weight -= nice_to_weight[task->nice + 20];
		task->sched->weight += nice_to_weight[nice + 20];
		spin_unlock(&task->sched->lock);

		task->nice = nice;
		task->weight = nice_to_weight[nice + 20];
		task->inv_weight = nice_to_wmult[nice + 20];
	}
}

void task_suspend(struct task_t * task)
{
	struct task_t * next;
	uint64_t now, detla;

	if(task)
	{
		if(task->status == TASK_STATUS_RUNNING)
		{
			now = ktime_to_ns(ktime_get());
			detla = now - task->start;

			task->time += detla;
			task->vtime += calc_delta_fair(task, detla);
			task->status = TASK_STATUS_SUSPEND;
			spin_lock(&task->sched->lock);
			list_add_tail(&task->list, &task->sched->suspend);
			spin_unlock(&task->sched->lock);

			next = scheduler_next_ready_task(task->sched);
			if(next)
			{
				scheduler_dequeue_task(task->sched, next);
				next->status = TASK_STATUS_RUNNING;
				next->start = now;
				scheduler_switch_task(task->sched, next);
			}
		}
		else
		{
			task->status = TASK_STATUS_SUSPEND;
			spin_lock(&task->sched->lock);
			list_add_tail(&task->list, &task->sched->suspend);
			spin_unlock(&task->sched->lock);
			scheduler_dequeue_task(task->sched, task);
		}
	}
}

void task_resume(struct task_t * task)
{
	if(task && (task->status == TASK_STATUS_SUSPEND))
	{
		task->vtime = task->sched->min_vtime;
		task->status = TASK_STATUS_RUNNING;
		spin_lock(&task->sched->lock);
		list_del_init(&task->list);
		spin_unlock(&task->sched->lock);
		scheduler_enqueue_task(task->sched, task);
	}
}


static void idle_task(struct task_t * task, void * data)
{
	while(1)
	{
	}
}


void schedule()
{
	struct scheduler_t * sched = scheduler_self();
	struct task_t * next, * self = task_self();
	uint64_t now = ktime_to_ns(ktime_get());
	uint64_t detla = now - self->start;

	self->time += detla;
	self->vtime += calc_delta_fair(self, detla);

	cli();
	current->flags &= ~NEED_SCHEDULE;
	//color_printk(RED,BLACK,"#schedule:current(pid=%d vrun_time=%d pointer=%X),tsk(pid=%d vrun_time=%d pointer=%X)\n", current->pid, current->vrun_time, current, tsk->pid, tsk->vrun_time, tsk);

	if((int64_t)(self->vtime - sched->min_vtime) < 0)
	{
		self->start = now;
	}
	else
	{
		scheduler_enqueue_task(sched, self);
		next = scheduler_next_ready_task(sched);
		scheduler_dequeue_task(sched, next);
		next->status = TASK_STATUS_RUNNING;
		next->start = now;
		if(likely(next != self)) {
			scheduler_switch_task(sched, next);
		}
			
	}

	sti();
}

void start_idle_task(void)
{
	struct scheduler_t * sched = scheduler_self();
	struct task_t * task, * next;
	int cpu = 0;

	void * stack;

	sched = &__sched[cpu];
	//stksz = CONFIG_TASK_SIZE;

	init_mm.pgd = (pgd_t *)mmu_get_tlb_base_addr();
	task = init_task[0];
	task->mm = &init_mm;
	task->status = TASK_STATUS_RUNNING;

	wait_queue_init(&task->wait_childexit, NULL);
	init_list_head(&task->list);
	task->preempt_count = 0;

	RB_CLEAR_NODE(&task->node);
	spin_lock(&sched->lock);
	list_add_tail(&task->list, &sched->suspend);
	init_list_head(&task->child_list);
	init_list_head(&task->child_node);
	task->nice = 26;
	task->weight = 3;
	task->inv_weight = 1431655765;
	sched->weight += task->weight;
	spin_unlock(&sched->lock);

	task->name = strdup("idle");
	task->status = TASK_STATUS_SUSPEND;
	task->start = ktime_to_ns(ktime_get());
	task->time = 0;
	task->vtime = 0;
	task->sched = sched;

	task_resume(task);

	next = scheduler_next_ready_task(sched);
	if(next)
	{
		sched->running = next;
		scheduler_dequeue_task(sched, next);
		next->status = TASK_STATUS_RUNNING;
		next->start = ktime_to_ns(ktime_get());
		//scheduler_switch_task(sched, next);
		current = next;
	}
}

void do_init_sched()
{
	struct task_t *task;
	struct scheduler_t * sched;

	memset(&__sched,0,sizeof(struct scheduler_t) * NR_CPUS);
	
	for(int i = 0; i < NR_CPUS; i++)
	{
		sched = &__sched[i];

		spin_lock_init(&sched->lock);
		spin_lock(&sched->lock);
		sched->ready = RB_ROOT_CACHED;
		init_list_head(&sched->suspend);
		sched->running = NULL;
		sched->min_vtime = 0;
		sched->weight = 0;
		spin_unlock(&sched->lock);
	}
	
	start_idle_task();
}
