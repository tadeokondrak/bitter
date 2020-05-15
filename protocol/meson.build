wayland_protocols_dep = dependency('wayland-protocols')
wayland_protocols_dir = wayland_protocols_dep.get_pkgconfig_variable('pkgdatadir')

wayland_scanner_dep = dependency('wayland-scanner')
wayland_scanner_bin = wayland_scanner_dep.get_pkgconfig_variable('wayland_scanner')

protocols = [
  wayland_protocols_dir / 'stable/xdg-shell/xdg-shell.xml',
]

protocols_src = []
protocols_inc = []

foreach protocol : protocols
  protocols_src += custom_target(
    protocol.underscorify() + '_c',
    input: protocol,
    output: '@BASENAME@-protocol.c',
    command: [wayland_scanner_bin, 'private-code', '@INPUT@', '@OUTPUT@'],
  )

  protocols_inc += custom_target(
    protocol.underscorify() + '_h',
    input: protocol,
    output: '@BASENAME@-protocol.h',
    command: [wayland_scanner_bin, 'server-header', '@INPUT@', '@OUTPUT@'],
  )
endforeach

protocols_lib = static_library(
  'protocols',
  [protocols_src, protocols_inc],
)

protocols_dep = declare_dependency(
  link_with: protocols_lib,
  sources: protocols_inc,
)
