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

#ifdef CS2DLUA_TARGET_WIN

#include <stdio.h>

#include <windows.h>

#ifndef CS2DJIT_EXE

#include "cs2djitbase.h"

BOOL APIENTRY DllMain(HINSTANCE _, DWORD reason, LPVOID __)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		int result;
		wchar_t *moduleName;
		FILE *fileExe;

		/* Windows API quirks -_- */
		moduleName = malloc(sizeof(wchar_t) * 32768);
		if (moduleName == NULL)
			return 0;
		
		if (GetModuleFileNameW(NULL, moduleName, 32767) == 0)
		{
			free(moduleName);
			return 0;
		}

		/* Open executable */
		fileExe = _wfopen(moduleName, L"rb");
		free(moduleName);
		if (fileExe == NULL)
			return 0;

		/* Init */
		result = cs2djit_init((size_t) GetModuleHandleA(NULL), fileExe);
		fclose(fileExe);
		return result;
	}

	return 1;
}

#else

static wchar_t g_TempBuffer[32768]; /* uh */

int main(int argc, char* argv[])
{
	wchar_t *lastSlash, *dedicatedFile = NULL, *currentDir = NULL;
	void *otherProcMem = NULL, *loadLibrary;
	size_t dedicatedLen, dirLen;
	HANDLE dedicated = INVALID_HANDLE_VALUE;
	HANDLE otherThread = INVALID_HANDLE_VALUE;
	DWORD exitCode;
	STARTUPINFOW startupInfo;
	PROCESS_INFORMATION processInfo;

	/* Windows API UTF-16 is bad */
	memset(g_TempBuffer, 0, 32768 * sizeof(wchar_t));
	if (GetModuleFileNameW(NULL, g_TempBuffer, 32767) == 0)
		return 1;
	
	/* LoadLibraryA address */
	loadLibrary = (void *) GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
	
	/* Setup cs2d_dedicated.exe paths */
	lastSlash = wcsrchr(g_TempBuffer, '/');
	if (lastSlash == NULL)
		lastSlash = wcsrchr(g_TempBuffer, '\\');
	if (lastSlash == NULL)
		lastSlash = g_TempBuffer - 1;
	lastSlash[1] = 0;

	/* Copy current directory */
	dirLen = wcslen(g_TempBuffer);
	if (dirLen > 0)
	{
		currentDir = malloc(dirLen * sizeof(wchar_t) + 1);
		currentDir[dirLen] = 0;
		memcpy(currentDir, g_TempBuffer, dirLen * sizeof(wchar_t));
	}

	/* Setup CS2D dedicated path */
	dedicatedLen = dirLen + sizeof("cs2d_dedicated.exe") + 2;
	dedicatedFile = malloc(dedicatedLen * sizeof(wchar_t));
	if (dedicatedFile == NULL)
		return 1;
	swprintf(dedicatedFile, dedicatedLen, L"%ls%ls", currentDir ? currentDir : L"", L"cs2d_dedicated.exe");

	/* Setup Startup Info */
	memset(&startupInfo, 0, sizeof(STARTUPINFOW));
	startupInfo.cb = sizeof(STARTUPINFOW);
	startupInfo.dwFlags = STARTF_USESTDHANDLES;
	startupInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
	startupInfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	startupInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);

	/* Start process */
	fprintf(stderr, "Launching cs2d_dedicated.exe with path: %ls\n", dedicatedFile);
	if (CreateProcessW(
		dedicatedFile, /* lpApplicationName */
		GetCommandLineW(), /* lpCommandLine */
		NULL, /* lpProcessAttributes */
		NULL, /* lpThreadAttributes */
		1, /* bInheritHandles */
		CREATE_SUSPENDED, /* dwCreationFlags */
		NULL, /* lpEnvironment */
		currentDir, /* lpCurrentDirectory */
		&startupInfo, /* lpStartupInfo */
		&processInfo /* lpProcessInformation */
	) == 0)
	{
		// fputs("Failed to start cs2d_dedicated.exe", stderr);
		fprintf(stderr, "Failed to start cs2d_dedicated.exe: %d\n", (int) GetLastError());
		free(currentDir);
		free(dedicatedFile);
		return 1;
	}

	/* Free some memory */
	free(currentDir); currentDir = NULL;
	free(dedicatedFile); dedicatedFile = NULL;

	/* Alloc memory to target process */
	otherProcMem = VirtualAllocEx(processInfo.hProcess, NULL, 1024, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (otherProcMem == NULL)
	{
		fputs("Cannot allocate memory in process\n", stderr);
		TerminateProcess(processInfo.hProcess, 1);
		return 1;
	}

	/* Write the DLL name */
	WriteProcessMemory(processInfo.hProcess, otherProcMem, "cs2djit", 8, NULL);

	/* Load DLL */
	otherThread = CreateRemoteThread(processInfo.hProcess, NULL, 0, loadLibrary, otherProcMem, 0, NULL);
	if (otherThread == INVALID_HANDLE_VALUE)
	{
		fputs("Cannot inject cs2djit.dll\n", stderr);
		VirtualFreeEx(processInfo.hProcess, otherProcMem, 1024, MEM_RELEASE);
		TerminateProcess(processInfo.hProcess, 1);
		return 1;
	}

	/* Start process */
	/* TODO: Handle Ctrl+C and pass it to cs2d_dedicated */
	WaitForSingleObject(otherThread, INFINITE);
	CloseHandle(otherThread);
	VirtualFreeEx(processInfo.hProcess, otherProcMem, 1024, MEM_RELEASE);
	ResumeThread(processInfo.hThread);
	WaitForSingleObject(processInfo.hProcess, INFINITE);

	/* Exit */
	GetExitCodeProcess(processInfo.hProcess, &exitCode);
	return exitCode;
}

#endif /* CS2DJIT_EXE */

#endif /* CS2DLUA_TARGET_WIN */
