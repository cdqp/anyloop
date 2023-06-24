project(
	'openao',
	'c',
)

json_dep = dependency('json-c')
gsl_dep= dependency('gsl')

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
	dependencies: [json_dep, gsl_dep],
	include_directories: incdir
)

