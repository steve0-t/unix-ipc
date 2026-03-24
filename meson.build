project('ipc_showcase', 'c')

add_project_arguments('-O3', language: 'c')

inc = include_directories('include')

libhelpers = static_library('helpers', 'src/helpers.c', include_directories: inc)

executable('proc_p1', 'src/proc_p1.c', link_with: libhelpers, include_directories: inc)
executable(
  'proc_serv2',
  'src/proc_serv2.c',
  link_with: libhelpers,
  include_directories: inc,
)
executable('proc_p2', 'src/proc_p2.c', link_with: libhelpers, include_directories: inc)
executable('proc_d', 'src/proc_d.c', link_with: libhelpers, include_directories: inc)
executable('proc_t', 'src/proc_t.c', link_with: libhelpers, include_directories: inc)
executable('zadanie', 'src/zadanie.c', link_with: libhelpers, include_directories: inc)