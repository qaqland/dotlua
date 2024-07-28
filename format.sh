#!/bin/sh

stylua tests/

clang-format -i main.c
