project('dotlua', 'c', license: 'GPL-3.0-only', version: '0.0.2')

bash = dependency('bash')
lua = dependency('lua5.4')

lua_so_path = lua.get_variable(pkgconfig: 'libdir') + '/liblua.so'

src = files('main.c')
subdir('lua')

add_project_arguments(
    '-DPROJ_VERSION="@0@"'.format(meson.project_version()),
    '-DPROJ_LUA_SO_PATH="@0@"'.format(lua_so_path),
    language: 'c',
)

dotlua = shared_module(
    'dotlua',
    src,
    name_prefix: '',  # no lib prefix
    dependencies: [bash, lua],
    include_directories: ['/usr/lib/bash/'],
    install: true,
)

bats = find_program('bats', required: false)

if bats.found()
    test(
        'bats tests',
        bats,
        args: ['tests', '--tap'],
        depends: [dotlua],
        protocol: 'tap',
        verbose: true,
        workdir: meson.project_source_root(),
    )
else
    warning('skip tests')
endif
