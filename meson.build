project(
	'openao',
	'c',
)


json_dep = dependency('json-c')
cblas_dep = dependency('gsl')

executable('openao', 'src/openao.c',
	install: true,
	dependencies: [json_dep, cblas_dep]
)

