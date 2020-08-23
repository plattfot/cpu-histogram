project('cpu-hist', 'cpp',
        default_options: ['cpp_std=c++17', 'b_ndebug=if-release'],
        version: run_command(find_program('cat'), 'VERSION').stdout().strip(),
        license: 'MIT')


configure_file(
  output: 'version.hpp',
  input: 'src/version.hpp.in',
  configuration:
    {
      'CPU_HIST_VERSION': meson.project_version(),
    })
configuration_include = include_directories('.')

fmt = dependency('fmt', version: '>=6')
jsoncpp = dependency('jsoncpp', version: '>=1.8')
executable('cpu-hist', 'src/main.cpp', 'src/cpu.cpp',
           include_directories: configuration_include,
           dependencies: [fmt, jsoncpp],
           install: true)
