bitter_src = files(
    'input_device.c',
    'main.c',
    'node.c',
    'output.c',
    'server.c',
    'view.c',
    'xdg_shell.c',
)

bitter_bin = executable(
  'bitter',
  bitter_src,
  dependencies: [
    wlroots_dep,
    wayland_server_dep,
    xkbcommon_dep,
    protocols_dep,
  ],
  install: true,
)
