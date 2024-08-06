setup() {
	BATS_LIB_PATH=/usr/lib/bats
	bats_load_library bats-support
	bats_load_library bats-assert

	# change work directory to tests
	cd $BATS_TEST_DIRNAME
	enable -f ../builddir/dotlua.so dotlua
}

teardown() {
	enable -d dotlua
}
