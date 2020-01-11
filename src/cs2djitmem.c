/*
 * Copyright (c) 2020 Miku AuahDark
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <stdio.h>

#include "cs2djittarget.h"

#ifdef CS2DLUA_TARGET_WIN

#include <Windows.h>

int cs2djit_mark_as_write(void *start, size_t len)
{
	DWORD oldProtect;

	if (VirtualProtect(start, len, PAGE_READWRITE, &oldProtect))
		/* Success */
		return (int) oldProtect;
	
	/* Fail */
	return -1;
}

int cs2djit_unmark_as_write(void *start, size_t len, int access)
{
	DWORD dummy;

	if (VirtualProtect(start, len, (DWORD) access, &dummy))
		/* Success */
		return 1;

	/* Fail */
	return 0;
}

#else

#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>

static size_t g_pageSize = 0;

int cs2djit_mark_as_write(void *start, size_t len)
{
	if (!g_pageSize) g_pageSize = getpagesize();

	/* Align memory offset */
	len = (len + g_pageSize - 1) & (~(g_pageSize - 1)); /* Next pagesize */
	start = (void *) (((size_t) start) & (~(g_pageSize - 1))); /* Previous pagesize */

	if (mprotect(start, len, PROT_READ | PROT_WRITE))
		/* Fail */
		return -1;
	/* Success */
	return PROT_READ | PROT_EXEC;
}

int cs2djit_unmark_as_write(void *start, size_t len, int access)
{
	if (!g_pageSize) g_pageSize = getpagesize();

	/* Align memory offset */
	len = (len + g_pageSize - 1) & (~(g_pageSize - 1)); /* Next pagesize */
	start = (void *) (((size_t) start) & (~(g_pageSize - 1))); /* Previous pagesize */

	if (mprotect(start, len, access))
		/* Fail */
		return 0;
	
	/* Success */
	return 1;
}

#endif
