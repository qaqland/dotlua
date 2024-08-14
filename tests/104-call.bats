#!/usr/bin/env bats

load test_helper.bash

_zero() {
	echo zero
}

_two() {
	echo two "$1" "$2"
}

_pwd() {
	pwd
}

exit_1() {
	exit 1
}

@test "sh.call function" {
	run dotlua -s 'sh.call("_zero")'
	assert_output 'zero'
}

@test "sh.call with parameters" {
	run dotlua -s 'sh.call("_two", "foo", 123)'
	assert_output 'two foo 123'
}

@test "sh.call retval" {
	run dotlua -s 'sh.call("exit_1")'
	assert_failure 1
}

@test "sh.call not builtin" {
	run dotlua -s 'sh.call("_pwd")'
	assert_output -p 'tests'
}

@test "sh.call builtin pwd" {
	# pwd: unknow shell function
	# skip "not implement"
	run dotlua -s 'sh.call("pwd")'
	assert_output -p 'tests'
}

@test "sh.call builtin cd" {
	dotlua -s 'sh.call("cd", "lua")'
	run dotlua -s 'sh.call("pwd", "-P")'
	assert_output -p 'tests/lua'
}

@test "sh.call binrary" {
	run dotlua -s 'sh.call("git", "--version")'
	assert_output -p 'git version'
}
