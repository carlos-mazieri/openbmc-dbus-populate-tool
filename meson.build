project('openbmc-dbus-populate-tool', 'cpp',
  default_options: [
                    'buildtype=debugoptimized',
                    'cpp_std=c++17',
                    'warning_level=3',
                    'werror=true'
                   ],
  license: 'MIT',
  version: '1.0')


conf = configuration_data()
conf.set('PRINT_DATA_STRUCTURES', 1)

configure_file(output: 'config.h', configuration: conf)

deps = [ dependency('sdbusplus'),
       ]


sdbuspp = find_program('sdbus++')

executable(
    'dbus-populate-tool',
    'main.cpp',
    'clone_service.cpp',
    'emulate_service.cpp',
    dependencies: [deps],
    install: true
)
