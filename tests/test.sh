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

testHello() {
    output=$(dotlua tests/hello.lua)
    assertEquals 'Hello World!' "$output"
}

testGetShellVar1() {
    a=foo
    output=$(dotlua tests/get_shell_var.lua)
    assertEquals 'a=foo' "$output"
}

testGetShellVar2() {
    a=
    output=$(dotlua tests/get_shell_var.lua)
    assertEquals 'a=' "$output"
}

testGetShellVar3() {
    unset a
    output=$(dotlua tests/get_shell_var.lua)
    assertContains "$output" 'nil'
}

testSetShellVar1() {
    a=1
    dotlua tests/set_shell_var.lua
    assertEquals "2" "$a"
}

testSetShellVar2() {
    unset a
    dotlua tests/set_shell_var.lua
    assertEquals "2" "$a"
}

testSetShellVar3() {
    export a=1
    dotlua tests/set_shell_var.lua
    assertEquals "2" "$a"
}

testCallShellFunction1() {
    call_zero() {
        echo call_zero
    }
    output=$(dotlua tests/call_shell_fn_1.lua)
    assertEquals 'call_zero' "$output"
}

testCallShellFunction3() {
    call_pwd() {
        pwd
    }
    output=$(dotlua tests/call_shell_fn_3.lua)
    assertContains "$output" 'dotlua'
}

testCallLuaFunction() {
    dotlua tests/bind_lua_func.lua
    output=$(add_2_numbers 1 2)
    assertEquals '3' "$output"
}

source "$(which shunit2)"
