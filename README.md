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

### DONE

- dotlua xxx.lua
- dotlua -f function

### TODO

- dotlua xxx.lua -- args
- dotlua -s "string lua"

- lua c api fix
- valgrind check
- clang format

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

TODO

## references

TODO

- <http://git.savannah.gnu.org/cgit/bash.git/>
- <https://www.codingwiththomas.com/blog/a-lua-c-api-cheat-sheet>
- <https://github.com/cjungmann/bash_builtin>
- <https://github.com/alfredopalhares/LuaBash>
- <https://stackoverflow.com/questions/5406858/difference-between-unset-and-empty-variables-in-bash>
