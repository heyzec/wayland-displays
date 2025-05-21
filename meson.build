project('wayland-displays', ['cpp', 'c'])

# External dependencies
gtkdep = dependency('gtk+-3.0')
spdlog = dependency('spdlog')
yaml = dependency('yaml-cpp')
zlib = dependency('epoxy')

# Generate wayland client protocol code (client_protocols)
subdir('protocols')
client_protocols_dep = declare_dependency(
  link_with: client_protocols,
  sources: client_protocol_headers,
)

# Generate resources (resources_c)
subdir('resources')

# In-repo dependencies
subdir('lib/argparse')
argparse_dep = declare_dependency(include_directories: argparse_headers)
subdir('lib/togglegroup')
togglegroup_dep = declare_dependency(include_directories: togglegroup_headers, link_with: togglegroup)

# Headers and sources for wayland-displays
src_includes = include_directories('src')
lib_includes = include_directories('lib')
subdir('src') # (src_files)

# Executable
executable('wayland-displays',
  sources: [
    src_files,
    resources_c,
  ],
  # TODO: Try to get rid of lib_includes (argparse_dep and togglegroup_dep should have been sufficient)
  include_directories: [src_includes, lib_includes],
  dependencies: [
    # External dependencies
    gtkdep,
    spdlog,
    yaml,
    zlib,
    # In-repo dependencies
    client_protocols_dep,
    argparse_dep,
    togglegroup_dep,
  ],
  install: true,
)
