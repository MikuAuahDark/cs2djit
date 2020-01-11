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

Put `libcs2djit.so` and `cs2djit.sh` to your CS2D server directory.

Or just use CMake to compile it and be happy.

### Windows

TODO

Running
-----

### Linux

Simply run `cs2djit.sh`

```sh
bash cs2djit.sh
```

### Windows

TODO
