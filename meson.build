project(
    'dotlua',
    'c',
    license: 'GPL-3.0-only',
    version: '0.0.1',

)

add_project_arguments(
    '-DVERSION="@0@"'.format(meson.project_version()),
    language: 'c',
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