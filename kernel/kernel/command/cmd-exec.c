/*
 * kernel/command/cmd-exec.c
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

#include <command/command.h>
#include <sizes.h>
#include <vfs.h>
#include <types.h>
#include <unistd.h>
#include <stdio.h>
#include <printk.h>
#include <task.h>
#include <sys/err.h>
#include <ptrace.h>
#include <lib.h>
#include <linkage.h>
#include <schedule.h>
#include <sched.h>

static void usage(void)
{
	printf("usage:\r\n");
	printf("    exec <file>\r\n");
}
char name_buf[VFS_MAX_PATH] = {0};

static void run(char *name)
{
	struct pt_regs *regs = (struct pt_regs *)((unsigned long)current + STACK_SIZE - sizeof(struct pt_regs));
	current->flags &= ~PF_KTHREAD;
	printf("name = %s\n",name);
	if(IS_ERR_VALUE(do_execve(regs, name, NULL, NULL))){
		printf("do_execve error");
		return 1;
	}

	//TODO:考虑CPSR
	asm	volatile(
		"mov	sp, %0		\n\t"
		"b		ret_system_call\n\t"
		:
		:"r"(regs)
		:"memory"
	);
}
static int exec_file(const char * filename)
{
	struct stat st;
	char fpath[VFS_MAX_PATH];
	char * buf;
	u64_t i, n;
	int fd;

	strcpy(fpath, filename);
	if(sys_stat(fpath, &st) < 0)
	{
		printf("exec: %s: No such file or directory\r\n", fpath);
		return -1;
	}

	if(S_ISDIR(st.st_mode))
	{
		printf("exec: %s: Is a directory\r\n", fpath);
		return -1;
	}
	strlcpy(name_buf, filename, VFS_MAX_PATH);
	kernel_thread(run, name_buf, CLONE_FS | CLONE_SIGHAND);
	return 0;
}

static int do_exec(int argc, char ** argv)
{
	int i;

	if(argc != 2)
	{
		usage();
		return -1;
	}

	if(exec_file(argv[1]) != 0)
		return -1;
	return 0;
}

static struct command_t cmd_exec = {
	.name	= "exec",
	.desc	= "run a program",
	.usage	= usage,
	.exec	= do_exec,
};

static __init void exec_cmd_init(void)
{
	register_command(&cmd_exec);
}

static __exit void exec_cmd_exit(void)
{
	unregister_command(&cmd_exec);
}

command_initcall(exec_cmd_init);
command_exitcall(exec_cmd_exit);
