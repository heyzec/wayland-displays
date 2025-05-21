wayland_client = dependency('wayland-client')
wayland_scanner = find_program('wayland-scanner')

wayland_scanner_code = generator(
  wayland_scanner,
  output: '@BASENAME@.c',
  arguments: ['private-code', '@INPUT@', '@OUTPUT@'],
)

wayland_scanner_client = generator(
  wayland_scanner,
  output: '@BASENAME@.h',
  arguments: ['client-header', '@INPUT@', '@OUTPUT@'],
)

protocols = [
  'wlr-output-management-unstable-v1.xml',
]

client_protocol_srcs = []
client_protocol_headers = []

foreach xml: protocols
  client_protocol_srcs += wayland_scanner_code.process(xml)
  client_protocol_headers += wayland_scanner_client.process(xml)
endforeach

client_protocols = static_library(
  'client_protocols',
  client_protocol_srcs + client_protocol_headers,
  dependencies: [wayland_client],
)
