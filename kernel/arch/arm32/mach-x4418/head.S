/*
 * start.S
 *
 * Copyright(c) 2007-2019 Jianjun Jiang <8192542@qq.com>
 * Official site: http://xboot.org
 * Mobile phone: +86-18665388956
 * QQ: 8192542
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */


/*
 * Exception vector table
 */
.text
	.arm

	.global _start
_start:
	b reset
	ldr pc, _vector_und
	ldr pc, _vector_swi
	ldr pc, _vector_pabt
	ldr pc, _vector_dabt
	ldr pc, _vector_addrexcptn
	ldr pc, _vector_irq
	ldr pc, _vector_fiq

_vector_und:
	.word vector_und
_vector_swi:
	.word vector_swi
_vector_pabt:
	.word vector_pabt
_vector_dabt:
	.word vector_dabt
_vector_addrexcptn:
	.word vector_addrexcptn
_vector_irq:
	.word vector_irq
_vector_fiq:
	.word vector_fiq

/*
 * The actual reset code
 */
reset:
	/* Set cpu to svc32 mode and disable interrupt */
	mrs r0, cpsr
	bic r0, r0, #0x1f
	orr r0, r0, #0xd3
	msr cpsr, r0

	/* Enable neon/vfp unit */
	mrc p15, #0, r1, c1, c0, #2
	orr r1, r1, #(0xf << 20)
	mcr p15, #0, r1, c1, c0, #2
	mov r1, #0
	mcr p15, #0, r1, c7, c5, #4
	mov r0, #0x40000000
	fmxr fpexc, r0

	/* Cache init */
	mrc p15, 0, r0, c0, c0, 0
	and r1, r0, #0x00f00000
	and r2, r0, #0x0000000f
	orr r2, r2, r1, lsr #20-4
	cmp r2, #0x30
	mrceq p15, 0, r0, c1, c0, 1
	orreq r0, r0, #0x6
	mcreq p15, 0, r0, c1, c0, 1

	/* Invalidate L1 I/D */
	mov r0, #0
	mcr p15, 0, r0, c8, c7, 0
	mcr p15, 0, r0, c7, c5, 0

	/* Disable mmu stuff and caches */
	mrc p15, 0, r0, c1, c0, 0
	bic r0, r0, #0x00002000
	bic r0, r0, #0x00000007
	bic r0, r0, #0x00000002
	orr r0, r0, #0x00000800
	mcr p15, 0, r0, c1, c0, 0

	/* Set Vector Base Address Register */
	ldr r0, =_start
	mcr p15, 0, r0, c12, c0, 0
	mrc p15, 0, r0, c1, c0, 0
	bic r0, #(1<<13)
	mcr p15, 0, r0, c1, c0, 0

	/* Initialize stacks */
	mrs r0, cpsr
	bic r0, r0, #0x1f
	orr r1, r0, #0x1b
	msr cpsr_cxsf, r1
	ldr sp, _stack_und_start

	bic r0, r0, #0x1f
	orr r1, r0, #0x17
	msr cpsr_cxsf, r1
	ldr sp, _stack_abt_start

	bic r0, r0, #0x1f
	orr r1, r0, #0x12
	msr cpsr_cxsf, r1
	ldr sp, _stack_irq_start

	bic r0, r0, #0x1f
	orr r1, r0, #0x11
	msr cpsr_cxsf, r1
	ldr sp, _stack_fiq_start

	bic r0, r0, #0x1f
	orr r1, r0, #0x13
	msr cpsr_cxsf, r1
	ldr sp, _stack_srv_end

	/* Copyself to link address */
	adr r0, _start
	ldr r1, =_start
	cmp r0, r1
	beq 1f
	ldr r0, _image_start
	adr r1, _start
	ldr r2, _image_end
	sub r2, r2, r0
	bl memcpy
1:	nop

	/* Clear bss section */
	ldr r0, _bss_start
	ldr r2, _bss_end
	sub r2, r2, r0
	mov r1, #0
	bl memset

	/* Call _main */
	ldr r1, =_main
	mov pc, r1
_main:
	mov r0, #0
	mov r1, #0
	mov fp, #0
	bl main
	b _main


/*
 * The location of section
 */
 	.align 4
_image_start:
	.long __image_start
_image_end:
	.long __image_end
_data_start:
	.long __data_start
_data_end:
	.long __data_end
_bss_start:
	.long __bss_start
_bss_end:
	.long __bss_end
_stack_und_start:
	.long __stack_und_start
_stack_abt_start:
	.long __stack_abt_start
_stack_irq_start:
	.long __stack_irq_start
_stack_fiq_start:
	.long __stack_fiq_start
_stack_srv_end:
	.long __stack_srv_end
