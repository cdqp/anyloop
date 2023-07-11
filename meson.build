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
	'libaylp/logging.c',
	'libaylp/block.c',
	'libaylp/config.c',
	'libaylp/anyloop.c',
	'devices/device.c',
	'devices/delay.c',
	'devices/file_sink.c',
	'devices/logger.c',
	'devices/stop_after_count.c',
	'devices/udp_sink.c',
	'devices/vonkarman_stream.c',
]

incdir = include_directories('libaylp')

executable('anyloop', project_source_files,
	install: true,
	dependencies: deps,
	include_directories: incdir
)

shared_library('plugin_example',
	['contrib/plug/plugin_example.c', 'libaylp/logging.c'],
	name_prefix: '',
	install: true,
	dependencies: deps,
	install_dir: 'plugins',
	include_directories: incdir
)

shared_library('menable5',
	['contrib/plug/menable5.c', 'libaylp/logging.c'],
	name_prefix: '',
	install: true,
	dependencies: deps,
	install_dir: 'plugins',
	include_directories: incdir
)

