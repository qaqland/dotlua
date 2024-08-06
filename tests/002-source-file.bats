#!/usr/bin/env bats

load test_helper.bash

@test "dotlua filename" {
	run dotlua lua/001-hello.lua
	assert_output 'Hello World!'
}

@test "dotlua filename arguments" {
	run dotlua lua/002-hello-args.lua 1 2 3
	assert_output 'Hello 1 2 3'
}

@test "dotlua filename -- argu ments (ignore first --)" {
	run dotlua lua/002-hello-args.lua -- 1 2 3
	assert_output 'Hello 1 2 3'
}

@test "dotlua filename argu -- ments (preserve middle)" {
	run dotlua lua/002-hello-args.lua 1 -- 2 3
	assert_output 'Hello 1 -- 2 3'
}
