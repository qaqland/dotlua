#!/usr/bin/env bats

load test_helper.bash

@test "sh.delete var" {
	foo=bar
	dotlua -s 'sh.delete("foo")'
	run echo $foo
	assert_output ''
}

@test "sh.delete env" {
	export foo=bar
	dotlua -s 'sh.delete("foo")'
	run bash -c 'echo $foo'
	assert_output ''
}
