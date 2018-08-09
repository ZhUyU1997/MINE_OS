#ifndef __ASM_OFFSETS_H__
#define __ASM_OFFSETS_H__

#define S_R0 0 /* offsetof(struct pt_regs, ARM_r0)	@ */
#define S_R1 4 /* offsetof(struct pt_regs, ARM_r1)	@ */
#define S_R2 8 /* offsetof(struct pt_regs, ARM_r2)	@ */
#define S_R3 12 /* offsetof(struct pt_regs, ARM_r3)	@ */
#define S_R4 16 /* offsetof(struct pt_regs, ARM_r4)	@ */
#define S_R5 20 /* offsetof(struct pt_regs, ARM_r5)	@ */
#define S_R6 24 /* offsetof(struct pt_regs, ARM_r6)	@ */
#define S_R7 28 /* offsetof(struct pt_regs, ARM_r7)	@ */
#define S_R8 32 /* offsetof(struct pt_regs, ARM_r8)	@ */
#define S_R9 36 /* offsetof(struct pt_regs, ARM_r9)	@ */
#define S_R10 40 /* offsetof(struct pt_regs, ARM_r10)	@ */
#define S_FP 44 /* offsetof(struct pt_regs, ARM_fp)	@ */
#define S_IP 48 /* offsetof(struct pt_regs, ARM_ip)	@ */
#define S_SP 52 /* offsetof(struct pt_regs, ARM_sp)	@ */
#define S_LR 56 /* offsetof(struct pt_regs, ARM_lr)	@ */
#define S_PC 60 /* offsetof(struct pt_regs, ARM_pc)	@ */
#define S_PSR 64 /* offsetof(struct pt_regs, ARM_cpsr)	@ */
#define S_OLD_R0 68 /* offsetof(struct pt_regs, ARM_ORIG_r0)	@ */
#define S_FRAME_SIZE 72 /* sizeof(struct pt_regs)	@ */


#endif
