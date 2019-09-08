project('cpu-hist', 'cpp',
        default_options: ['cpp_std=c++17'],
        version: '0.1.0',
        license: 'MIT')

docopt = dependency('docopt', version: '>=0.6')
fmt = dependency('fmt', version: '>=6')
jsoncpp = dependency('jsoncpp', version: '>=1.8')
executable('cpu-hist', 'src/main.cpp', 'src/cpu.cpp',
           cpp_args: ['-DCPU_HIST_VERSION="'+meson.project_version()+'"'],
           dependencies: [fmt, docopt, jsoncpp])
