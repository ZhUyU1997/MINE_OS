/*
 * kernel/command/cmd-cat.c
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
#include <malloc.h>
#include <stdio.h>

static void usage(void)
{
	printf("usage:\r\n");
	printf("    cat <file> ...\r\n");
}

static int cat_file(const char * filename)
{
	struct stat st;
	char fpath[VFS_MAX_PATH];
	char * buf;
	u64_t i, n;
	int fd;

	strcpy(fpath, filename);
	if(sys_stat(fpath, &st) < 0)
	{
		printf("cat: %s: No such file or directory\r\n", fpath);
		return -1;
	}

	if(S_ISDIR(st.st_mode))
	{
		printf("cat: %s: Is a directory\r\n", fpath);
		return -1;
	}

	fd = sys_open(fpath, O_RDONLY, 0);
	if(fd < 0)
	{
		printf("cat: %s: Can not open\r\n", fpath);
		return -1;
	}

	buf = malloc(SZ_64K);
	if(!buf)
	{
		printf("cat: Can not alloc memory\r\n");
		sys_close(fd);
		return -1;
	}

	while((n = sys_read(fd, buf, SZ_64K)) > 0)
	{
		for(i = 0; i < n; i++)
			putchar(buf[i]);
	}
	printf("\r\n");

	free(buf);
	sys_close(fd);

	return 0;
}

static int do_cat(int argc, char ** argv)
{
	int i;

	if(argc == 1)
	{
		usage();
		return -1;
	}

	for(i = 1; i < argc; i++)
	{
		if(cat_file(argv[i]) != 0)
			return -1;
	}
	return 0;
}

static struct command_t cmd_cat = {
	.name	= "cat",
	.desc	= "show the contents of a file",
	.usage	= usage,
	.exec	= do_cat,
};

static __init void cat_cmd_init(void)
{
	register_command(&cmd_cat);
}

static __exit void cat_cmd_exit(void)
{
	unregister_command(&cmd_cat);
}

command_initcall(cat_cmd_init);
command_exitcall(cat_cmd_exit);
