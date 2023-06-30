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

if get_option('readonly').enabled()
    conf.set('DEFAULT_SERVICE_READONLY', 1)
endif

default_service_file = get_option('servicefile')
if default_service_file != ''
    conf.set('DEFAULT_SERVICE_FILE', default_service_file)
endif


configure_file(output: 'config.h', configuration: conf)

deps = [ dependency('sdbusplus'),
       ]


sdbuspp = find_program('sdbus++')

executable(
    'dbus-populate-tool',
    'main.cpp',
    'clone_service.cpp',
    'emulate_service.cpp',
    'methods_map.cpp',
    dependencies: [deps],
    install: true
)

