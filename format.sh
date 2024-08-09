#!/usr/bin/env bash

stylua tests/lua

shfmt --write tests/*.{bats,bash}

# clang-format --verbose
find . -name '*.c' -a \( ! -path '*builddir*' \) \
    -exec clang-format -i {} \;

meson format -i -r -c meson.format

cloc . --exclude-dir builddir --force-lang "Bourne Again Shell,bats"
