/*
 * exception.c
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
#include <types.h>
#include <ptrace.h>

static void show_regs(struct pt_regs * regs)
{
	int i;

	printf("pc : [<%08lx>] lr : [<%08lx>] cpsr: %08lx\r\n", regs->ARM_pc, regs->ARM_lr, regs->ARM_cpsr);
	printf("sp : %08lx svc sp : %08lx\r\n", regs->ARM_sp, regs);
	for(i = 12; i >= 0; i--)
	{
		printf("r%-2d: %08lx ", i, regs->uregs[i]);
		if(i % 2 == 0)
			printf("\r\n");
	}
	printf("\r\n");
}

void arm32_do_undefined_instruction(struct pt_regs * regs)
{
	show_regs(regs);
	backtrace(regs->ARM_fp);
	regs->ARM_pc += 4;
}

void arm32_do_software_interrupt(struct pt_regs * regs)
{
	show_regs(regs);
	backtrace(regs->ARM_fp);
	regs->ARM_pc += 4;
}

void arm32_do_prefetch_abort(struct pt_regs * regs)
{
	show_regs(regs);
	backtrace(regs->ARM_fp);
	regs->ARM_pc += 4;
}

void arm32_do_data_abort(struct pt_regs * regs)
{
	show_regs(regs);
	backtrace(regs->ARM_fp);
	regs->ARM_pc += 4;
}

void arm32_do_irq(struct pt_regs * regs)
{
	interrupt_handle_exception(regs);
}

void arm32_do_fiq(struct pt_regs * regs)
{
	interrupt_handle_exception(regs);
}
