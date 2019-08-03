/*
 * kernel/command/cmd-rmdir.c
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
#include <vfs.h>
#include "sys/dirent.h"
#include "errno.h"

static void usage(void)
{
	printf("usage:\r\n");
	printf("    rmdir DIRECTORY...\r\n");
}

static int do_rmdir(int argc, char ** argv)
{
	int ret = 0;

	if(argc < 2)
	{
		usage();
		return -1;
	}

	for(int i = 1; i < argc; i++)
	{
		struct stat st;
		if(sys_stat(argv[i], &st) < 0 || !S_ISDIR(st.st_mode))
		{
			printf("rmdir: failed to remove directory %s: Not a directory\n", argv[i]);
			continue;
		}

		int err = sys_rmdir(argv[i]);
		if(err == -ENOTEMPTY) {
			printf("rmdir: failed to remove directory %s: Directory not empty\n", argv[i]);
			continue;
		}

		if(err != 0)
		{
			ret = -1;
			printf("rmdir: failed to remove directory %s\n", argv[i]);
			continue;
		}
	}
	return ret;
}

static struct command_t cmd_rmdir = {
	.name	= "rmdir",
	.desc	= "remove empty directories",
	.usage	= usage,
	.exec	= do_rmdir,
};

static __init void rmdir_cmd_init(void)
{
	register_command(&cmd_rmdir);
}

static __exit void rmdir_cmd_exit(void)
{
	unregister_command(&cmd_rmdir);
}

command_initcall(rmdir_cmd_init);
command_exitcall(rmdir_cmd_exit);
