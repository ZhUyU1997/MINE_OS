/*
 * kernel/command/cmd-touch.c
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

static void usage(void)
{
	printf("usage:\r\n");
	printf("    touch [FILE]...\r\n");
}

static int do_touch(int argc, char ** argv)
{
	char fpath[VFS_MAX_PATH];
	char ** v;
	int pflag = 0;
	int c = 0;
	int ret = 0;
	int i;

	if(!(v = malloc(sizeof(char *) * argc)))
		return -1;

	for(i = 1; i < argc; i++)
	{
		v[c++] = argv[i];
	}

	if(c == 0)
	{
		usage();
		free(v);
		return -1;
	}

	for(i = 0; i < c; i++)
	{
		if(sys_open(v[i], O_CREAT, 0755) < 0)
		{
			ret = -1;
			printf("touch: failed to touch file %s\r\n", v[i]);
		}
	}
	free(v);

	return ret;
}

static struct command_t cmd_touch = {
	.name	= "touch",
	.desc	= "Update the access and modification times of each FILE to the current time",
	.usage	= usage,
	.exec	= do_touch,
};

static __init void touch_cmd_init(void)
{
	register_command(&cmd_touch);
}

static __exit void touch_cmd_exit(void)
{
	unregister_command(&cmd_touch);
}

command_initcall(touch_cmd_init);
command_exitcall(touch_cmd_exit);
