/*
 * kernel/core/subsys.c
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

#include <json.h>
#include <core/driver.h>
#include <core/initcall.h>

extern unsigned char __dtree_start;
extern unsigned char __dtree_end;

static void subsys_init_dt(void)
{
	char * json;
	int len = 0;
	len = &__dtree_end - &__dtree_start + 1;
	json = &__dtree_start;
	probe_device(json, len, "hello");
}

static void subsys_init_rootfs(void)
{
	mount_fs("/", CONFIG_BLOCK_DEV, "FAT32");
	mount_fs("/sys", NULL, "sys");
}

static __init void subsys_init(void)
{
	subsys_init_dt();
	subsys_init_rootfs();
}
subsys_initcall(subsys_init);
