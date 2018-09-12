#ifndef __TRACE_H__
#define __TRACE_H__

#define KERN_EMERG		(0x001)	/*紧急事件消息，系统崩溃之前提示，表示系统不可用*/
#define KERN_ALERT		(0x002)	/*报告消息，表示必须立即采取措施*/
#define KERN_CRIT		(0x004)	/*临界条件，通常涉及严重的硬件或软件操作失败*/
#define KERN_ERR		(0x008)	/*错误条件，驱动程序常用KERN_ERR来报告硬件的错误*/
#define KERN_WARNING	(0x010)	/*警告条件，对可能出现问题的情况进行警告*/
#define KERN_NOTICE 	(0x020)	/*正常但又重要的条件，用于提醒。常用于与安全相关的消息*/
#define KERN_INFO 		(0x040)	/*提示信息，如驱动程序启动时，打印硬件信息*/
#define KERN_DEBUG 		(0x080)	/*调试级别的消息*/
#define KERN_ALL		(0xffffffff)
static unsigned int trace_mask = KERN_ALL&(~(KERN_DEBUG));

#define trace(msk, fmt, ...) do { \
			if (trace_mask & (msk)) \
				printf("trace: " fmt "\n", ##__VA_ARGS__); \
		} while (0)
#define set_trace_mask(mask) do { \
			trace |= mask;\
		} while (0)

#define clr_trace_mask(mask) do { \
			trace &= ~(mask);\
		} while (0)
			
//trace(KERN_DEBUG, 
//trace(KERN_EMERG, 
//trace(KERN_ALERT, 
//trace(KERN_CRIT, 
//trace(KERN_ERR, 
//trace(KERN_WARNING, 
//trace(KERN_NOTICE, 
//trace(KERN_INFO, 
//trace(KERN_DEBUG, 
//trace(KERN_ALL, 
#endif
