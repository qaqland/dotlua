#!/usr/bin/env bats

load test_helper.bash

@test "sh.get var" {
	foo=bar
	run dotlua -s 'print(sh.get("foo"))'
	assert_output 'bar'
}

@test "sh.get empty" {
	foo=
	run dotlua -s 'print(sh.get("foo"))'
	assert_output ''
}

@test "sh.get env" {
	export foo=bar
	run dotlua -s 'print(sh.get("foo"))'
	assert_output 'bar'
}

@test "sh.get nil" {
	run dotlua -s 'return sh.get("foo") == nil'
	assert_success
}
