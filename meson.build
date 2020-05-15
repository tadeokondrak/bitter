project(
    'bitter',
    'c',
    default_options: [
        'c_std=gnu11',
        'warning_level=3',
    ],
)

add_project_arguments(
    [
        '-DWLR_USE_UNSTABLE',
        '-D_POSIX_C_SOURCE=200809L',
        '-Wno-unused-parameter',
        '-Wno-pedantic',
    ],
    language: 'c',
)

wlroots_dep = dependency('wlroots')
wayland_server_dep = dependency('wayland-server')
xkbcommon_dep = dependency('xkbcommon')

subdir('protocol')
subdir('src')
