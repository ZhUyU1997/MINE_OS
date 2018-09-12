
/***************************************************
*		��Ȩ����
*
*	������ϵͳ��Ϊ��MINE
*	�ò���ϵͳδ����Ȩ������ӯ�����ӯ��ΪĿ�Ľ��п�����
*	ֻ�������ѧϰ�Լ���������ʹ��
*
*	������������Ȩ������Ȩ���������У�
*
*	��ģ�����ߣ�	����
*	EMail:		345538255@qq.com
*
*
***************************************************/

#ifndef __SOFTIRQ_H__
#define __SOFTIRQ_H__

#include <lib.h>

#define TIMER_SIRQ	(1 << 0)

extern unsigned long softirq_status;

struct softirq {
	void (*action)(void * data);
	void * data;
};

extern struct softirq softirq_vector[BITS_PER_LONG];


void register_softirq(int nr, void (*action)(void * data), void * data);
void unregister_softirq(int nr);

void set_softirq_status(unsigned long status);
unsigned long get_softirq_status();

void softirq_init();

#endif
