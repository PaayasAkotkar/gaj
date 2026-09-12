project('n','cpp',version:'1.0.0')

inc = include_directories('src','src/', 'example' )

cpp_compiler = meson.get_compiler('cpp')

cpp_files = run_command(
  'python3', '-c',
  'from pathlib import Path; print(" ".join(str(p) for p in Path("src").rglob("*.cpp")))',
  check: true,
).stdout().strip().split()

hpp_files = run_command(
  'python3', '-c',
  'from pathlib import Path; print(" ".join(str(p) for p in Path("src").rglob("*.hpp")))',
  check: true,
).stdout().strip().split()
test_hpp_files = run_command(
  'python3', '-c',
  'from pathlib import Path; print(" ".join(str(p) for p in Path("example").rglob("*.hpp")))',
  check: true,
).stdout().strip().split()

executable(
  'app',
  ['main.cpp'] + cpp_files + hpp_files+test_hpp_files,
  include_directories: inc,
  install: false,
)
