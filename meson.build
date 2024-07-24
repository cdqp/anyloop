project(
	'anyloop',
	'c',
	default_options: ['c_std=gnu17', 'warning_level=3', 'optimization=3']
	#default_options: ['warning_level=everything', 'optimization=3']
	# this is worth occasionally turning on to check
)

c = meson.get_compiler('c')
deps = [
	dependency('json-c'),
	dependency('gsl'),
	dependency('threads'),
	c.find_library('dl')
]

project_source_files = [
	'libaylp/anyloop.c',
	'libaylp/logging.c',
	'libaylp/block.c',
	'libaylp/config.c',
	'libaylp/pretty.c',
	'libaylp/thread_pool.c',
	'libaylp/xalloc.c',
	'libaylp/profile.c',
	'devices/center_of_mass.c',
	'devices/clamp.c',
	'devices/device.c',
	'devices/delay.c',
	'devices/file_sink.c',
	'devices/logger.c',
	'devices/matmul.c',
	'devices/pid.c',
	'devices/poke.c',
	'devices/remove_piston.c',
	'devices/stop_after_count.c',
	'devices/test_source.c',
	'devices/udp_sink.c',
	'devices/vonkarman_stream.c',
]

incdir = include_directories('libaylp')

executable('anyloop', project_source_files,
	install: true,
	install_dir: '/opt/anyloop',
	dependencies: deps,
	include_directories: incdir,
	export_dynamic: true
)

shared_library('plugin_example', 'contrib/plugin_example.c',
	name_prefix: '',
	install: true,
	install_dir: '/opt/anyloop',
	dependencies: deps,
	include_directories: incdir,
	override_options: 'b_lundef=false'
)

