# dotlua

dotlua help to run lua code with bash shell

## usage

```bash
#!/usr/bin/env bash
enable -f dotlua.so dotlua

dotlua abuild.lua

enable -d dotlua
```

functions are unstable now, see tests for detail

## PLAN

| status | item                      |
| ------ | ------------------------- |
| done   | `dotlua xxx.lua`          |
| done   | `dotlua -f function args` |
|        | `dotlua xxx.lua -- args`  |
|        | `dotlua -s "string lua"`  |
|        | lua c api fix             |
|        | valgrind check            |
|        | clang format              |

## setup

```bash
# apk add bash (?)
# apk add meson
# apk add lua5.4-dev bash-dev

meson setup builddir
meson compile -C builddir
```

### test

```bash
# apk add shunit2

./tests/test.sh
```

## LICENSE

GPL-3.0-only

## Acknowledge

This project is highly inspired by [LuaBash][1].

Start to DIY your own built from this [blog][2] and get more builtin example and details in [bash_builtin][3] and [bash][4] itself.

[1]: https://github.com/alfredopalhares/LuaBash
[2]: https://blog.dario-hamidi.de/a/build-a-bash-builtin
[3]: https://github.com/cjungmann/bash_builtin
[4]: http://git.savannah.gnu.org/cgit/bash.git/
