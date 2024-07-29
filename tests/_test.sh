#!/usr/bin/env bash

oneTimeSetUp() {
	enable -f builddir/dotlua.so dotlua
}

oneTimeTearDown() {
	# shunit2's bug:
	# https://github.com/kward/shunit2/issues/112
	[ "$_shunit_name_" = 'EXIT' ] && return 0
	enable -d dotlua
}

testDotLuaPrefix() {
	enable -d dotlua
	export DOTLUA_PREFIX="dotlua"
	enable -f builddir/dotlua.so dotlua
	a=foo
	output=$(dotlua tests/000-prefix.lua)
	assertEquals 'a=foo' "$output"
	unset DOTLUA_PREFIX
	enable -d dotlua
	enable -f builddir/dotlua.so dotlua
}

testHello() {
	output=$(dotlua tests/001-hello.lua)
	assertEquals 'Hello World!' "$output"
}

testHelloArgs1() {
	output=$(dotlua tests/002-hello-args.lua 1 2 3)
	assertEquals 'Hello 1 2 3' "$output"
}

testHelloArgs2() {
	output=$(dotlua tests/002-hello-args.lua -- 1 2 3)
	assertEquals 'Hello 1 2 3' "$output"
}

testHelloArgs3() {
	output=$(dotlua tests/002-hello-args.lua 1 -- 2 3)
	assertEquals 'Hello 1 -- 2 3' "$output"
}

testGetShellVar1() {
	a=foo
	output=$(dotlua tests/003-get-shell-var.lua)
	assertEquals 'a=foo' "$output"
}

testGetShellVar2() {
	a=
	output=$(dotlua tests/003-get-shell-var.lua)
	assertEquals 'a=' "$output"
}

testGetShellVar3() {
	unset a
	output=$(dotlua tests/003-get-shell-var.lua 2>&1)
	assertContains "$output" 'nil'
}

testSetShellVar1() {
	a=1
	dotlua tests/004-set-shell-var.lua
	assertEquals "2" "$a"
}

testSetShellVar2() {
	unset a
	dotlua tests/004-set-shell-var.lua
	assertEquals "2" "$a"
}

testSetShellVar3() {
	export a=1
	dotlua tests/004-set-shell-var.lua
	assertEquals "2" "$a"
}

testCallShellFunction1() {
	call_zero() {
		echo call_zero
	}
	output=$(dotlua tests/005-call-shell.lua)
	assertEquals 'call_zero' "$output"
}

testCallShellFunction2() {
	call_two() {
		echo call_two "$1" "$2"
	}
	output=$(dotlua tests/006-call-shell-args.lua)
	assertEquals 'call_two string 123' "$output"
}

testCallShellFunction3() {
	call_pwd() {
		pwd
	}
	output=$(dotlua tests/007-call-shell.lua)
	assertContains "$output" 'dotlua'
}

testCallLuaFunction1() {
	dotlua tests/008-bind-lua-args.lua
	output=$(add_2_numbers 1 2)
	assertEquals '3' "$output"
}

testCallLuaFunction2() {
	dotlua tests/009-bind-lua.lua
	output=$(hello)
	assertEquals 'hello' "$output"
}

testCallLuaString() {
	output=$(dotlua -s 'print("hello")')
	assertEquals 'hello' "$output"
}

source "$(which shunit2)"
