# DotLua

DotLua help to run lua code with bash shell

## Usage

```bash
#!/usr/bin/env bash
enable -f dotlua.so dotlua

dotlua abuild.lua

enable -d dotlua
```

functions are unstable and, see tests for detail

```bash
# export foo
sh.put("foo")
# export foo=bar
sh.put("foo", "bar")

# foo=bar
sh.set("foo", "bar")
# $foo
sh.get("foo")

# unset foo
sh.delete("foo")

# bind lua func to shell
sh.bind("add_2_number")

# call shell func from lua
sh.call("call_two", 1, 2)
# check result
rc = sh.call("call_zero")

# catch stdout/stderr?
# TODO
```

## Plan

| status | item                      |
| ------ | ------------------------- |
| done   | `dotlua xxx.lua`          |
| done   | `dotlua xxx.lua -- args`  |
| done   | `dotlua -s "string lua"`  |
| done   | `dotlua -f function args` |
|        | call shell built-ins      |
|        | call shell `cd`           |
|        | lua c api fix             |
|        | valgrind check            |
|        | clang format              |

## Setup

```bash
# apk add bash (?)
# apk add meson
# apk add lua5.4-dev bash-dev

meson setup builddir
meson compile -C builddir
```

### Test

```bash
# apk add shunit2

./tests/_test.sh
```

## LICENSE

GPL-3.0-only

## Acknowledge

This project is highly inspired by [LuaBash][1].

Start to DIY your own built from this [blog][2] and get more builtin example
and details in [bash_builtin][3] and [bash][4] itself.

[1]: https://github.com/alfredopalhares/LuaBash
[2]: https://blog.dario-hamidi.de/a/build-a-bash-builtin
[3]: https://github.com/cjungmann/bash_builtin
[4]: http://git.savannah.gnu.org/cgit/bash.git/
