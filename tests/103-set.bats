#!/usr/bin/env bats

load test_helper.bash

@test "sh.set var" {
	foo=
	dotlua -s 'sh.set("foo", 1)'
	assert_equal "$foo" 1
}

@test "sh.set one parameter (failure)" {
	run dotlua -s 'sh.set("foo")'
	assert_failure
}

@test "sh.set new" {
	dotlua -s 'sh.set("foo", 1)'
	assert_equal "$foo" 1
}

@test "sh.set env" {
	export foo
	dotlua -s 'sh.set("foo", 1)'
	run bash -c 'echo $foo'
	assert_output 1
}
