project(
    'dotlua',
    'c',
    license: 'GPL-3.0-or-later',
)

bash = dependency('bash')
lua = dependency('lua5.4')

shared_module(
    'dotlua',
    'main.c',
    name_prefix: '', # no lib prefix
    dependencies: [bash, lua],
    include_directories: ['/usr/lib/bash/'],
)