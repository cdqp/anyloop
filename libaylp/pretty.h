#ifndef AYLP_PRETTY_H_
#define AYLP_PRETTY_H_

#include <stdio.h>
#include <gsl/gsl_matrix.h>

// pretty-print gsl types
void pretty_vector(gsl_vector *v);
void pretty_matrix(gsl_matrix *m);

#endif

