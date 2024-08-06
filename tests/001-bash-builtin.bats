#!/usr/bin/env bats

load test_helper.bash

@test "enable builtin dotlua" {
	run enable
	assert_output -p 'dotlua'
}

@test "remove builtin dotlua" {
	enable -d dotlua
	run enable
	refute_output -p 'dotlua'

	# teardown would remove it
	enable -f ../builddir/dotlua.so dotlua
}
