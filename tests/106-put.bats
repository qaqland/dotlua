#!/usr/bin/env bats

load test_helper.bash

@test "sh.put export foo" {
	unset foo
	foo=bar
	dotlua -s 'sh.put("foo")'
	run bash -c 'echo $foo'
	assert_output 'bar'
}

@test "sh.put export foo (bash)" {
	unset foo
	foo=bar
	export foo
	run bash -c 'echo $foo'
	assert_output 'bar'
}

@test "sh.put export foo=bar" {
	unset foo
	dotlua -s 'sh.put("foo", "bar")'
	run bash -c 'echo $foo'
	assert_output 'bar'
}

@test "sh.put export foo=bar (bash)" {
	unset foo
	export foo=bar
	run bash -c 'echo $foo'
	assert_output 'bar'
}
