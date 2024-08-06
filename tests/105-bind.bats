#!/usr/bin/env bats

load test_helper.bash

@test "sh.bind call function" {
	dotlua lua/009-bind-lua.lua
	dotlua -s 'sh.bind("hello")'
	run hello
	assert_output 'hello'
}

@test "sh.bind bind with new name" {
	dotlua lua/009-bind-lua.lua
	dotlua -s 'sh.bind("new-hello", hello)'
	run new-hello
	assert_output 'hello'
}

@test "sh.bind bind with new function" {
	dotlua -s 'sh.bind("hello", function() print("hello") end)'
	run hello
	assert_output 'hello'
}

@test "sh.bind call with arguments" {
	dotlua lua/008-bind-lua-args.lua
	dotlua -s 'sh.bind("add_2_numbers")'
	run add_2_numbers 1 2
	assert_output 3
}

@test "sh.bind return code" {
	skip "not implement"
	dotlua lua/010-bind-lua-ret.lua
	run exit_1
	assert_failure 1
}
