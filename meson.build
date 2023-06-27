project(
	'openao',
	'c',
	default_options: ['warning_level=everything', 'optimization=3']
)

json_dep = dependency('json-c')
gsl_dep = dependency('gsl')
deps = [json_dep, gsl_dep]

project_source_files = [
	'src/block.c',
	'src/config.c',
	'src/device.c',
	'src/logging.c',
	'src/openao.c',
	'src/devices/delay.c',
	'src/devices/file_sink.c',
	'src/devices/logger.c',
	'src/devices/udp_sink.c',
	'src/devices/vonkarman_stream.c',
]

incdir = include_directories('src')

executable('openao', project_source_files,
	install: true,
	dependencies: deps,
	include_directories: incdir
)

shared_library('plugin_example',
	['contrib/plugin_example.c', 'src/logging.c'],
	name_prefix: '',
	install: true,
	install_dir: 'plugins',
	include_directories: incdir
)

