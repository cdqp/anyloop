project(
	'openao',
	'c',
)

json_dep = dependency('json-c')
cblas_dep = dependency('gsl')

project_source_files = [
	'src/config.c',
	'src/device.c',
	'src/logging.c',
	'src/openao.c',
	'src/devices/logger.c',
	'src/devices/vonkarman_screen.c',
]

incdir = include_directories('src')

executable('openao', project_source_files,
	install: true,
	dependencies: [json_dep, cblas_dep],
	include_directories: incdir
)

