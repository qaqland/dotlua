#!/usr/bin/env bats

load test_helper.bash

@test "dotlua -rf function print (system)" {
	run dotlua -rf print 'Hello World!'
	assert_output 'Hello World!'
}

@test "dotlua -rf function hello (custom)" {
	dotlua -s 'function hello(n) print("Hello " .. n) end'
	run dotlua -rf hello 111
	assert_output 'Hello 111'
}

@test "dotlua -rf function return not nil" {
	dotlua -s 'function exit_1() return 1 end'
	run dotlua -rf exit_1
	assert_failure
}

@test "dotlua -rf function -- argu ments" {
	dotlua -s 'function hello(n) print("Hello " .. n) end'
	run dotlua -rf hello -- 111
	assert_output 'Hello 111'
}

@test "dotlua -rf function argu -- ments" {
	dotlua -s 'function hello(n1, n2, n3) print(n1 .. n2 ..n3) end'
	run dotlua -rf hello 111 -- 222
	assert_output '111--222'
}
