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
#include <stdlib.h>
#include <string.h>

#ifndef luaI_openlib
#define luaI_openlib luaL_openlib
#endif

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include "cs2djitbase.h"
#include "cs2djitmem.h"
#include "cs2djittarget.h"

#define CS2DLUAWRAP_NOFUNCDEF

/* Platform selection */
#if defined(CS2DLUA_TARGET_WIN)
#	include "../luawrap/address/win_1.0.1.4.h"
#elif defined(CS2DLUA_TARGET_MAC)
#	error "Patches welcome for macOS!"
#elif defined(CS2DLUA_TARGET_LINUX)
#	include "../luawrap/address/linux_1.0.1.4.h"
#else
#	error "Unknown target!"
#endif


static unsigned int crc32Table[] = {
	/* CRC polynomial 0xedb88320 */
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
	0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
	0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
	0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
	0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
	0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
	0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
	0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
	0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
	0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
	0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
	0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
	0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
	0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
	0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
	0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
	0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
	0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
	0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
	0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
	0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
	0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
	0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
	0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
	0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
	0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
	0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
	0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

static unsigned int updateCRC32(unsigned char octet, unsigned int crc)
{
	return (crc32Table[(crc ^ octet) & 0xff] ^ ((crc) >> 8));
}

static size_t minValue(size_t a, size_t b)
{
	return a > b ? b : a;
}

static size_t maxValue(size_t a, size_t b)
{
	return a > b ? a : b;
}

static void patchJumpAddress(size_t addr, size_t funcAddr)
{
	size_t calcAddress;
	unsigned char *target = (unsigned char *) addr;

#ifdef CS2DLUA_TARGET_MAC
#	error "TODO: calculate 64-bit jmp address"
#else
	/* jmp rel32 <the actual LuaJIT address> */
	target[0] = 0xE9; /* jmp rel32 */

	calcAddress = funcAddr - addr - 5;
	/* Little-endian address */
	target[1] = calcAddress & 0xFF;
	target[2] = (calcAddress >> 8) & 0xFF;
	target[3] = (calcAddress >> 16) & 0xFF;
	target[4] = (calcAddress >> 24) & 0xFF;
#endif
}

int cs2djit_init(size_t baseAddress, FILE *exe)
{
	unsigned char fileByte;
	unsigned int crcExe = 0xFFFFFFFF;
	size_t *luaFuncPtr, minAddress = (size_t) -1, maxAddress = 0, targetAddress, targetOffset;
	size_t i = (((size_t) &luaFunction.lua_newstate) - ((size_t) &luaFunction)) / sizeof(void*);
	int protAccess;

	/* Calculate CRC of the executable */
	while (fread(&fileByte, 1, 1, exe) == 1)
		crcExe = updateCRC32(fileByte, crcExe);

	crcExe = ~crcExe;
	if (crcExe == luaFunction.checksumDedicated)
		targetOffset = (size_t) luaFunction.dedicatedOffset;
	else if (crcExe == luaFunction.checksumClient)
		targetOffset = (size_t) luaFunction.clientOffset;
	else
	{
		/* Not found */
		fprintf(stdout, "CS2DJIT: No matching checksum for dedicated.\n");
		return 0;
	}

	/* Found */
	luaFuncPtr = (size_t *) &luaFunction;
	targetAddress = baseAddress + targetOffset;

	/* First pass: Determine range of address */
	for (; i < sizeof(LuaFunctionPointer)/sizeof(void*); i++)
	{
		size_t addr = targetAddress + luaFuncPtr[i];
		minAddress = minValue(addr, minAddress);
		maxAddress = maxValue(addr, maxAddress);
	}

	/* Set memory protection */
	protAccess = cs2djit_mark_as_write((void *) minAddress, maxAddress - minAddress + 6);
	if (protAccess == -1)
	{
		/* Cannot protect */
		fprintf(stdout, "CS2DJIT: Unable to set memory protection for patching.\n");
		return 0;
	}

	/* Second pass: Patch address */
#define PATCHJUMP(func) patchJumpAddress(targetAddress + (size_t) (luaFunction.func), (size_t) &func)
	PATCHJUMP(lua_newstate);
	PATCHJUMP(lua_close);
	PATCHJUMP(lua_newthread);
	PATCHJUMP(lua_atpanic);
	PATCHJUMP(lua_gettop);
	PATCHJUMP(lua_settop);
	PATCHJUMP(lua_pushvalue);
	PATCHJUMP(lua_remove);
	PATCHJUMP(lua_insert);
	PATCHJUMP(lua_replace);
	PATCHJUMP(lua_checkstack);
	PATCHJUMP(lua_xmove);
	PATCHJUMP(lua_isnumber);
	PATCHJUMP(lua_isstring);
	PATCHJUMP(lua_iscfunction);
	PATCHJUMP(lua_isuserdata);
	PATCHJUMP(lua_type);
	PATCHJUMP(lua_typename);
	PATCHJUMP(lua_equal);
	PATCHJUMP(lua_rawequal);
	PATCHJUMP(lua_lessthan);
	PATCHJUMP(lua_tonumber);
	PATCHJUMP(lua_tointeger);
	PATCHJUMP(lua_toboolean);
	PATCHJUMP(lua_tolstring);
	PATCHJUMP(lua_objlen);
	PATCHJUMP(lua_tocfunction);
	PATCHJUMP(lua_touserdata);
	PATCHJUMP(lua_tothread);
	PATCHJUMP(lua_topointer);
	PATCHJUMP(lua_pushnil);
	PATCHJUMP(lua_pushnumber);
	PATCHJUMP(lua_pushinteger);
	PATCHJUMP(lua_pushlstring);
	PATCHJUMP(lua_pushstring);
	PATCHJUMP(lua_pushvfstring);
	PATCHJUMP(lua_pushfstring);
	PATCHJUMP(lua_pushcclosure);
	PATCHJUMP(lua_pushboolean);
	PATCHJUMP(lua_pushlightuserdata);
	PATCHJUMP(lua_pushthread);
	PATCHJUMP(lua_gettable);
	PATCHJUMP(lua_getfield);
	PATCHJUMP(lua_rawget);
	PATCHJUMP(lua_rawgeti);
	PATCHJUMP(lua_createtable);
	PATCHJUMP(lua_newuserdata);
	PATCHJUMP(lua_getmetatable);
	PATCHJUMP(lua_getfenv);
	PATCHJUMP(lua_settable);
	PATCHJUMP(lua_setfield);
	PATCHJUMP(lua_rawset);
	PATCHJUMP(lua_rawseti);
	PATCHJUMP(lua_setmetatable);
	PATCHJUMP(lua_setfenv);
	PATCHJUMP(lua_call);
	PATCHJUMP(lua_pcall);
	PATCHJUMP(lua_cpcall);
	PATCHJUMP(lua_load);
	PATCHJUMP(lua_dump);
	PATCHJUMP(lua_yield);
	PATCHJUMP(lua_resume);
	PATCHJUMP(lua_status);
	PATCHJUMP(lua_gc);
	PATCHJUMP(lua_error);
	PATCHJUMP(lua_next);
	PATCHJUMP(lua_concat);
	PATCHJUMP(lua_getallocf);
	PATCHJUMP(lua_setallocf);
	PATCHJUMP(lua_getstack);
	PATCHJUMP(lua_getinfo);
	PATCHJUMP(lua_getlocal);
	PATCHJUMP(lua_setlocal);
	PATCHJUMP(lua_getupvalue);
	PATCHJUMP(lua_setupvalue);
	PATCHJUMP(lua_sethook);
	PATCHJUMP(lua_gethook);
	PATCHJUMP(lua_gethookmask);
	PATCHJUMP(lua_gethookcount);
	PATCHJUMP(luaopen_base);
	PATCHJUMP(luaopen_table);
	PATCHJUMP(luaopen_io);
	PATCHJUMP(luaopen_os);
	PATCHJUMP(luaopen_string);
	PATCHJUMP(luaopen_math);
	PATCHJUMP(luaopen_debug);
	PATCHJUMP(luaopen_package);
	PATCHJUMP(luaL_openlibs);
	PATCHJUMP(luaI_openlib); /* Should it be patched? */
	PATCHJUMP(luaL_register);
	PATCHJUMP(luaL_getmetafield);
	PATCHJUMP(luaL_callmeta);
	PATCHJUMP(luaL_typerror);
	PATCHJUMP(luaL_argerror);
	PATCHJUMP(luaL_checklstring);
	PATCHJUMP(luaL_optlstring);
	PATCHJUMP(luaL_checknumber);
	PATCHJUMP(luaL_optnumber);
	PATCHJUMP(luaL_checkinteger);
	PATCHJUMP(luaL_optinteger);
	PATCHJUMP(luaL_checkstack);
	PATCHJUMP(luaL_checktype);
	PATCHJUMP(luaL_checkany);
	PATCHJUMP(luaL_newmetatable);
	PATCHJUMP(luaL_checkudata);
	PATCHJUMP(luaL_where);
	PATCHJUMP(luaL_error);
	PATCHJUMP(luaL_checkoption);
	PATCHJUMP(luaL_ref);
	PATCHJUMP(luaL_unref);
	PATCHJUMP(luaL_loadfile);
	PATCHJUMP(luaL_loadbuffer);
	PATCHJUMP(luaL_loadstring);
	PATCHJUMP(luaL_newstate);
	PATCHJUMP(luaL_gsub);
	PATCHJUMP(luaL_findtable);
	PATCHJUMP(luaL_buffinit);
	PATCHJUMP(luaL_prepbuffer);
	PATCHJUMP(luaL_addlstring);
	PATCHJUMP(luaL_addstring);
	PATCHJUMP(luaL_addvalue);
	PATCHJUMP(luaL_pushresult);
#undef PATCHJUMP

	/* Reset memory protection status */
	if (cs2djit_unmark_as_write((void *) minAddress, maxAddress - minAddress + 6, protAccess) == 0)
	{
		/* Inconsistent state, too dangerous to continue! */
		fprintf(stdout, "CS2DJIT: Inconsistent state, exiting.\n");
		exit(1);
		return 0;
	}

	fprintf(stdout, "CS2DJIT: JIT power on!\n");
	return 1;
}
