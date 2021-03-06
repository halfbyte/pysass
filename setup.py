from distutils.core import setup, Extension

sass = Extension('sass',
                    sources = [
                      'sass.c', 
                      'libsass/sass_interface.cpp', 
                      'libsass/context.cpp', 
                      'libsass/document.cpp', 
                      'libsass/document_parser.cpp',
                      'libsass/eval_apply.cpp',
                      'libsass/functions.cpp',
                      'libsass/node.cpp',
                      'libsass/node_comparisons.cpp',
                      'libsass/values.cpp',
                      'libsass/prelexer.cpp'
                    ],
                    extra_compile_args = [
                      "-Wno-strict-prototypes", "-Wno-sign-compare", "-Wno-switch", "-Wno-shorten-64-to-32", "-Wno-unused-variable"
                    ]
)

setup (name = 'LibSass',
       version = '0.1',
       description = 'Python Bindings for libsass',
       ext_modules = [sass])