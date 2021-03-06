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

#include "cs2djittarget.h"

#ifdef CS2DLUA_TARGET_LINUX

#include <stdio.h>
#include <stdlib.h>

#include "cs2djitbase.h"

__attribute__ ((constructor)) void DllMain(void)
{
	int patchSuccess = 0;
	FILE *fileMaps = fopen("/proc/self/maps", "r");

	if (fileMaps)
	{
		// First 8 bytes only
		char addr[9];
		addr[8] = 0;

		if (fread(addr, 1, 8, fileMaps) == 8)
		{
			FILE *fileExe;
			size_t address;
			
			fileExe = fopen("/proc/self/exe", "rb");
			address = (size_t) strtoul(addr, NULL, 16);

			if (address)
				patchSuccess = cs2djit_init(address, fileExe);

			fclose(fileExe);
		}

		fclose(fileMaps);
	}

	if (patchSuccess == 0)
		fprintf(stdout, "CS2DJIT: Patch unsuccessful\n");
}

#endif /* CS2DLUA_TARGET_LINUX */
