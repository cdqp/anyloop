project(
	'anyloop',
	'c',
	default_options: ['c_std=gnu2x', 'warning_level=3', 'optimization=3']
	#default_options: ['warning_level=everything', 'optimization=3']
	# this is worth occasionally turning on to check
)

json_dep = dependency('json-c')
gsl_dep = dependency('gsl')
deps = [json_dep, gsl_dep]

project_source_files = [
	'src/block.c',
	'src/config.c',
	'src/device.c',
	'src/logging.c',
	'src/anyloop.c',
	'src/devices/delay.c',
	'src/devices/file_sink.c',
	'src/devices/logger.c',
	'src/devices/udp_sink.c',
	'src/devices/vonkarman_stream.c',
]

incdir = include_directories('include')

executable('anyloop', project_source_files,
	install: true,
	dependencies: deps,
	include_directories: incdir
)

shared_library('plugin_example',
	['contrib/plug/plugin_example.c', 'src/logging.c'],
	name_prefix: '',
	install: true,
	dependencies: deps,
	install_dir: 'plugins',
	include_directories: incdir
)

shared_library('menable5',
	['contrib/plug/menable5.c', 'src/logging.c'],
	name_prefix: '',
	install: true,
	dependencies: deps,
	install_dir: 'plugins',
	include_directories: incdir
)

