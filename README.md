CS2D JIT
=====

Replaces CS2D Lua engine with [LuaJIT](http://luajit.org/luajit.html) for faster server performance,
external Lua C modules support, and FFI support.

**Disclaimer: This only benefits server scripter. This is not a form of cheat/hack that can give
players unfair advantage in servers!**

I can't say it's stable. Use at your own risk. **If stability of your script and server is a
concern, don't use this!**

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
gcc -m32 -mno-sse -shared -o libcs2djit.so -I/usr/include/luajit-2.1 src/cs2djitbase.c src/cs2djitmem.c src/cs2djitlinux.c -Wl,--no-undefined -lluajit-5.1
```

You may want to adjust the include directories if you have LuaJIT in non-standard location.

Put `libcs2djit.so` and `cs2djit.sh` to your CS2D server directory.

Or just use CMake to compile it.

```sh
CFLAGS="-m32 -mno-sse" cmake -Bbuild -H. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$PWD/install
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
cmake -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-w64.cmake -Bbuild -H. -DCMAKE_INSTALL_PREFIX=$PWD/install
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

Any additional arguments are passed to cs2d_dedicated as-is.

### Windows

Note: Some antivirus mistakenly mark cs2djitwrapper.exe as unwanted software. Rest assured you already aware and decided to
use the prebuilt binaries anyway or you've compiled one yourself.

Place `cs2djitwrapper.exe` and `cs2djit.dll` at same folder as your CS2D server folder beside `cs2d_dedicated.exe`, then
simply run `cs2djitwrapper.exe`.

```
cs2djitwrapper.exe
```

Any additional arguments are passed to cs2d_dedicated.exe as-is.
