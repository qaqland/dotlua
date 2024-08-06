#!/usr/bin/env bats

load test_helper.bash

@test "dotlua -s string" {
	run dotlua -s 'print("Hello World!")'
	assert_output 'Hello World!'
}

@test "dotlua -s string ignored" {
	run dotlua -s 'print("Hello World!")' ignored
	assert_output 'Hello World!'
}
