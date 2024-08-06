#!/usr/bin/env bats

load test_helper.bash

@test "default is \"sh\"" {
	foo=bar
	run dotlua -s 'sh.get("foo")'
	assert_success
}

@test "default is \"sh\" (failure)" {
	foo=bar
	run dotlua -s 'dotlua.get("foo")'
	assert_failure
}

_setup() {
	enable -d dotlua
	export DOTLUA_PREFIX="dotlua"
	enable -f ../builddir/dotlua.so dotlua
	unset DOTLUA_PREFIX
}

@test "custom library name" {
	_setup
	foo=bar
	run dotlua -s 'dotlua.get("foo")'
	assert_success
}

@test "custom library name (failure)" {
	_setup
	foo=bar
	run dotlua -s 'sh.get("foo")'
	assert_failure
}
