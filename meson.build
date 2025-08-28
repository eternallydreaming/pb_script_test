project('pb_script_test', 'c')

sources = [
  'src/main.c',
  'src/utility.c',
  'src/lexer.c',
  'src/type_def.c',
  'src/value.c',
  'src/expr.c',
  'src/chunk.c',
  'src/bytecode.c',
  'src/parser.c',
  'src/registry.c',
  'src/vm.c',
]

cc = meson.get_compiler('c')
libm = cc.find_library('m', required: false)

executable(
  'pb_script_test',
  sources,
  include_directories: 'include/',
  dependencies: libm,
)