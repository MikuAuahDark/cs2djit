CS2D JIT
=====

Replaces CS2D Lua engine with [LuaJIT](http://luajit.org/luajit.html) for faster server performance,
external Lua C modules support, and FFI support.

**Disclaimer: 

Disclaimer
-----

* **This only benefits server scripter. This is not a form of cheat/hack that can give players unfair advantage in servers!**

* Use at your own risk. **If stability of your script and server is a concern, don't use this!**

* UnrealSoftware can ask me to remove this repository if according to them, this repository is dangerous. Although the
chance of this happening is unlikely as [this has been acknowledged by UnrealSoftware](https://www.unrealsoftware.de/forum_posts.php?post=428363&start=20#post428447).

Motivation
-----

LuaJIT was used in CS2D [0.1.2.6](http://www.unrealsoftware.de/forum_posts.php?post=390572), however
it's removed in [0.1.2.7](http://www.unrealsoftware.de/forum_posts.php?post=390972) due to stability
issue.

Because I'm one of the person who gets disappointed when CS2D revert back to Lua 5.1, I wrote
this program. This bring back LuaJIT support to CS2D by redirecting all Lua API calls in CS2D to
LuaJIT. The process is transparent, CS2D doesn't aware that the Lua has been replaced with LuaJIT.

Compilation
-----

### Linux

Make sure your GCC is set up with multiarch.

Install 32-bit LuaJIT development libraries. In Debian-based distro:

```sh
# If your host is 32-bit:
sudo apt install libluajit-5.1-dev
# If your host is 64-bit (enable multiarch support first!)
sudo apt install libluajit-5.1-dev:i386
```

Compile

```sh
gcc -m32 -shared -o libcs2djit.so -I/usr/include/luajit-2.1 src/cs2djitbase.c src/cs2djitmem.c src/cs2djitlinux.c -Wl,--no-undefined -lluajit-5.1
```

You may want to adjust the include directories if you have LuaJIT in non-standard location.

Put `libcs2djit.so` and `cs2djit.sh` to your CS2D server directory.

Or just use CMake to compile it.

```sh
CFLAGS=-m32 cmake -Bbuild -H. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$PWD/install
cmake --build build --target install
```

Binaries and scripts can be found in `install/bin`

### Windows

#### Cross-compiling from Linux

Assume Debian-based, it can be done with `mingw-w64` toolchain. This however does not cover how you compile
LuaJIT itself. Refer to [LuaJIT page](https://luajit.org/install.html#cross) on how to cross-compile one.

```sh
# Assume environment variable LUAJIT_DIR is location where the LuaJIT include and resulting libraries are
# and $PWD = current repository:
cmake -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-w64.cmake -Bbuild -H. -DCMAKE_INSTALL_PREFIX=$PWD/install -DCMAKE_BUILD_TYPE=Release
cmake --build build --target install
```

The resulting binaries can be found in `install/bin`. You may want to `strip` it manually first. It helps reducing antivirus
false alarms.

#### MSVC

It's also possible to use MSVC, but this is not recommended due CS2D uses GNU toolchain for some parts of its compilation
and any _funny_ problems caused by it can't, and won't be fixed.

```cmd
rem Assume you have Visual Studio toolchain, and environment variable LUAJIT_DIR is location where the
rem LuaJIT include and resulting libraries are, plus %CD% = current repository:
cmake -Bbuild -H. -DCMAKE_INSTALL_PREFIX=%CD%/install
cmake --build build --config Release --target install
```

The resulting binaries can be found in `install/bin`.

#### MinGW

The steps is more or less, similar to cross-compile from Linux above.

Running
-----

### Linux

Place `cs2djit.sh` and `libcs2djit.so` at same folder as your CS2D server folder beside `cs2d_dedicated`, then simply run
`cs2djit.sh`.

```sh
bash cs2djit.sh
```

Any additional arguments are passed to `cs2d_dedicated` as-is.

### Windows

Note: Some antivirus mistakenly mark `cs2djitwrapper.exe` as unwanted software. Rest assured you already aware and decided to
use the prebuilt binaries anyway or you've compiled one yourself.

Place `cs2djitwrapper.exe` and `cs2djit.dll` at same folder as your CS2D server folder beside `cs2d_dedicated.exe`, then
simply run `cs2djitwrapper.exe`.

```
cs2djitwrapper.exe
```

Any additional arguments are passed to `cs2d_dedicated.exe` as-is.

Design/How it Works
-----

If you aren't curious on how this program works then scroll up. Nothing to see here.

### Bootstrap

Since it must inject the jump pointer address to the LuaJIT shared library, then it must somehow hook it before
CS2D dedicated creates new Lua state. Easiest way to do this is prior the whole CS2D dedicated runs. Unfortunately
bootstrapping it is OS-specific.

#### Windows

Main file: `src/cs2djitwin.c`

Since Windows lack something like `LD_PRELOAD` environment variable in Linux, that means I have to inject `cs2djit.dll` into
the `cs2d_dedicated.exe` program.

##### `main` entry point

This entry point is executed if you run `cs2djitwrapper.exe`. It's quite simple.

* It loads up the current module path to `wchar_t[32768]`. That number is not specific. It's the longest possible path that
`GetModuleFileNameW` can return. [Unfortunately there's no way to get the size of it.](https://stackoverflow.com/q/805814)

* The variable `loadLibrary` contains the address of [`LoadLibraryA`](https://docs.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-loadlibrarya)
function. I can't simply use `loadLibrary = &LoadLibraryA` because `&LoadLibraryA` will be pointing to some stubs **in the
exe, not the DLL**. We'll use this later to inject `cs2djit.dll`.

* The next is setting up the paths which points to the current `cs2d_dedicated.exe`. Also set up some `STARTUPINFO`, reusing
the std handles as the parent.

* Then finally I start the `cs2d_dedicated.exe`, but **in suspended state**. This will give the wrapper exe time to inject the
`cs2djit.dll` as long as it needs.

* Next step is allocate new memory in the CS2D dedicated process and write string literal `"cs2djit"` on the returned address.

* To actually inject the `cs2djit.dll` to the process, the wrapper creates new thread at the CS2D dedicated process. Passing
the entry point of the thread to the address of `LoadLibraryA` as set from the `loadLibrary` variable and passing the pointer
allocated memory by `VirtualAllocEx` before as the opaque pointer. **TL;DR**: the thread simply call `LoadLibraryA("cs2djit")`.
This entry point effectively also starts the `DllMain` codepath.

* Then it resumes the CS2D dedicated process and waits until the dedicated server is closed.

##### `DllMain` entry point.

This entry point is executed when the remote thread runs `LoadLibraryA("cs2djit")` (see above).

* Usual Windows API quirks of `GetModuleFileNameW` with its infamous `wchar_t[32768]`. The calls to `GetModuleFileNameW` will
return full path to the CS2D dedicated.

* Then it opens the file and pass the `FILE*` and the executable base address to `cs2djit_init` which did all the trampoline stuff.

#### Linux

Main file: `src/cs2djitlinux.c` and `cs2djit.sh`.

The shell script is quite simple. It simply sets the `LD_PRELOAD` to `libcs2djit.so` which eventually injected to the
CS2D dedicated before it starts up.

The `DllMain` entry point in `cs2djitlinux.c` is also straightforward. It reads `/proc/self/maps` to get the CS2D dedicated base
address then it open `/proc/self/exe` to open the current `cs2d_dedicated` executable and pass it to `cs2djit_init`.

Very straightforward if you know about Linux `procfs`.

### `cs2djit_init`

Main file: `cs2djitbase.c`

Almost all the respective pointer addresses of the Lua function is hosted on the [`luawrap`](https://github.com/MikuAuahDark/cs2dluawrap)
Git submodule. It was my attempt to provide a dummy `lua51.dll` which can be used for external Lua C modules. Eventually I ditched it
because I think replacing the whole Lua 5.1.4 engine with LuaJIT is better.

So, if new CS2D update comes, then that repo needs to be updated with new function pointers.

* The first thing it do is to check the CRC32 of the game version string. If there's mismatch, then it's most likely this CS2D JIT
is incompatible with the CS2D dedicated used. This is not 100% foolproof though.

* Then it determines the range of the address that needs to be marked as read/write. I want to mark least memory as possible.

* It sets the memory ranges to read/write then perform the trampoline. For each Lua function listed, it writes `jmp` instruction, calculate
the relative address, then write the relative jump address to the CS2D dedicated memory.

* Then it reset the memory protection to its old values.

After all those steps is done, then the CS2D dedicated will start running. CS2D dedicated won't notice anything.
