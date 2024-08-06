#!/usr/bin/env bash

stylua tests/

shfmt --write tests/*.{bats,bash}

clang-format -i main.c
