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
#include <stdio.h>

static void usage(void)
{
	printf("usage:\r\n");
	printf("    write <file> <data>\r\n");
}

static int write_file(const char * filename, const char *data)
{
	struct stat st;
	char fpath[VFS_MAX_PATH];
	char * buf;
	u64_t i, n;
	int fd;

	strcpy(fpath, filename);
	if(sys_stat(fpath, &st) < 0)
	{
		printf("write: %s: No such file\n", fpath);
		return -1;
	}

	if(S_ISDIR(st.st_mode))
	{
		printf("write: %s: Is a directory\r\n", fpath);
		return -1;
	}

	fd = sys_open(fpath, O_RDONLY, 0);
	if(fd < 0)
	{
		printf("write: %s: Can not open\r\n", fpath);
		return -1;
	}

	sys_lseek(fd, 0, SEEK_END);
	if(sys_write(fd, data, strlen(data)) < 0)
	{
		printf("write: Can not write\r\n");
	}
	printf("\r\n");

	sys_close(fd);

	return 0;
}

static int do_write(int argc, char ** argv)
{
	int i;

	if(argc != 3)
	{
		usage();
		return -1;
	}

	write_file(argv[1], argv[2]);
	return 0;
}

static struct command_t cmd_write = {
	.name	= "write",
	.desc	= "write data to a file",
	.usage	= usage,
	.exec	= do_write,
};

static __init void write_cmd_init(void)
{
	register_command(&cmd_write);
}

static __exit void write_cmd_exit(void)
{
	unregister_command(&cmd_write);
}

command_initcall(write_cmd_init);
command_exitcall(write_cmd_exit);
