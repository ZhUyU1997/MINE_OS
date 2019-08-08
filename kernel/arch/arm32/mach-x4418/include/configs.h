#ifndef __OVERRIDE_CONFIGS_H__
#define __OVERRIDE_CONFIGS_H__

#define CONFIG_CPU_TLB_V7

#define CONFIG_PAGE_OFFSET			(0)
#define	CONFIG_TASK_SIZE			(0x40100000)
#define CONFIG_CODE_START_ADDR		(0x800000)
#define CONFIG_STACK_START_ADDR		(0xa00000)
#define CONFIG_BRK_START_ADDR		(0xc00000)
#define CONFIG_BLOCK_DEV			"card.sdhci-xl00500.2.p0"
#define CONFIG_FB_DEV				"fb-s5p4418.0"

#endif /* __OVERRIDE_CONFIGS_H__ */
