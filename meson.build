project('gtk-dashboard', 'c',
  version: '1.0.0',
  default_options: ['c_std=c11', 'warning_level=2']
)

gtk4_dep = dependency('gtk4')
json_glib_dep = dependency('json-glib-1.0')

executable('gtk-dashboard',
  files(
    'src/main.c',
    'src/app.c',
    'src/json_parser.c',
    'src/widget_factory.c',
    'src/style_manager.c',
    'src/shape_renderer.c'
  ),
  dependencies: [gtk4_dep, json_glib_dep, meson.get_compiler('c').find_library('m', required: false)],
  install: true
)
