resources_inputs = [
  'resources.xml',
  'layout.ui',
]

resources_c = custom_target(
  'resources_c',
  input: resources_inputs,
  output: 'resources.c',
  command: [
    'glib-compile-resources',
    '--target=@OUTPUT@',
    '--sourcedir=' + meson.current_source_dir(),
    '--generate-source',
    '@INPUT0@',
  ],
)
