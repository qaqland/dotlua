#!/bin/sh

stylua tests/

shfmt --write tests/_test.sh

clang-format -i main.c
