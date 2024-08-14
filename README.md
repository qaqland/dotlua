# dotlua

dotlua help to run lua code with bash shell

- Call lua function from shell and vice versa
- Get, set and export shell variables directly from lua

## Usage

```bash
$ enable -f path/to/dotlua.so dotlua

# dot lua
$ dotlua -s 'print(_VERSION)'

# remove
$ enable -d dotlua
```

It is recommended to start your bash script with `env`

```bash
#!/usr/bin/env bash
```

## Setup

```bash
# apk add bash (?)
# apk add meson
# apk add lua5.4-dev bash-dev

meson setup builddir
meson compile -C builddir
```

## CLI

### Source File

```bash
$ dotlua FILE [ARGUMENTS ...]
```

### Call Function

```bash
$ dotlua [-r] -f FUNCTION [ARGUMENTS ...]
```

Option `-r` allow you to call the raw function.

### Execute String

```bash
$ dotlua -s "LUA STRING"
```

### List Lua Bindings

```bash
$ dotlua -l [PATTERN]
```

## API

Set env `DOTLUA_PREFIX` to override default namespace `sh`

### `sh.put()`

```lua
-- export foo
sh.put("foo")

-- export foo=bar
sh.put("foo", "bar")
```

### `sh.set()`

```lua
-- foo=bar
sh.set("foo", "bar")
```

### `sh.get()`

```lua
-- $foo
sh.get("foo")
```

### `sh.delete()`

```lua
-- unset foo
sh.delete("foo")
```

### `sh.bind()`

```lua
sh.bind("add_2_number")
sh.bind("add-2-number", add_2_number)
```

Call lua function from shell after binding

### `sh.call()`

```lua
sh.call("call_two", 1, 2)
```

Call shell function from lua

### `sh.F`

Store function bindings

## TODO

- [x] `dotlua xxx.lua`
- [x] `dotlua xxx.lua -- args`
- [x] `dotlua -s "string lua"`
- [x] `dotlua -f function args`
- [x] `dotlua -l match`
- [x] call shell built-ins
- [x] call shell `cd`
- [x] lua c api fix
- [ ] valgrind check
- [x] clang format
- [ ] catch stdout/stderr?
- [x] lua function return
- [x] put (export)
- [x] delete (unset)

## Test

```bash
# apk add bats

bats tests
```

## LICENSE

Copyright (C) 2024 qaqland

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.

## Acknowledge

This project is highly inspired by [LuaBash][1].

Start to make your own builtin from this [blog][2] and get more builtin example
and details in [bash_builtin][3] and [bash][4] itself.

[1]: https://github.com/alfredopalhares/LuaBash
[2]: https://blog.dario-hamidi.de/a/build-a-bash-builtin
[3]: https://github.com/cjungmann/bash_builtin
[4]: http://git.savannah.gnu.org/cgit/bash.git/
