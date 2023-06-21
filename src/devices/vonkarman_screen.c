#include <math.h>
#include <gsl/gsl_math.h>
#include "openao.h"
#include "logging.h"
#include "vonkarman_screen.h"


/* von Kármán spectrum
 * see: https://doi.org/10.1364/AO.43.004527
 * and https://doi.org/10.1117/12.279029
 * L0 is outer scale length [m]
 * r0 is Fried parameter [m]
 * Gx and Gy are width and height of phase screen [m]
 * x and y are dimensionless (indices)
 * output is dimensionless (1/px)
 * Ivy's original MATLAB code:
 * karmanSpec = @(L0,r0,Gx,Gy,x,y) (0.15132*(Gx*Gy)^(-1/2)*r0^(-5/6) ...
 *     * ((x/Gx).^2+(y/Gy).^2+1/L0^2).^(-11/12));
 */
double karman_spec(double L0, double r0, double Gx, double Gy,
	size_t x, size_t y
){
	return (
		0.15132
		/ sqrt(Gx*Gy)
		/ powf(r0, 5.0/6.0)
		/ powf(gsl_hypot3(x/Gx, y/Gy, 1/L0), 11.0/6.0)
	);
}


void vonkarman_screen_init(struct oao_device *self)
{
	self->process = &vonkarman_screen_process;
	self->close = &vonkarman_screen_close;
	log_trace("vonkarman_screen initialized");
	log_trace("%f", karman_spec(2.0,0.02,50.0,50.0,0,1));
	return;
}


void vonkarman_screen_process(struct oao_device *self, struct oao_state *state)
{
	log_trace("vonkarman_screen processed");
	return;
}


void vonkarman_screen_close(struct oao_device *self)
{
	log_trace("vonkarman_screen closed");
	return;
}

