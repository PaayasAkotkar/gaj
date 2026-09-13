project('gaj', 'cpp', version : '1.0.0')

lib_inc = include_directories('src')

gaj_dep = declare_dependency(
  include_directories : lib_inc
)
meson.override_dependency('gaj_lib', gaj_dep)

if not meson.is_subproject()
  
  inc = include_directories('src', 'example')

  executable(
    'app',
    ['main.cpp'],
    include_directories : inc,
    dependencies : gaj_dep,
    install : false,
  )
endif
