#!/usr/bin/env bats

load test_helper.bash

@test "dotlua -l" {
	dotlua -s 'sh.bind("pr-int32", print)'
	dotlua -s 'sh.bind("pr-uint8", print)'
	dotlua -s 'sh.bind("pr-float", print)'
	run dotlua -l
	assert_line 'pr-int32'
	assert_line 'pr-uint8'
	assert_line 'pr-float'
}

@test "dotlua -l pattern" {
	dotlua -s 'sh.bind("pr-int32", print)'
	dotlua -s 'sh.bind("pr-uint8", print)'
	dotlua -s 'sh.bind("pr-float", print)'
	run dotlua -l uint
	assert_output 'pr-uint8'
}
