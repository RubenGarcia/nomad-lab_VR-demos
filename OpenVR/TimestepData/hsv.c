#include <math.h>
#include "hsv.h"
//http://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-in-range-0-255-for-both
void hsv2rgb(float *hsv, float *rgb) {
	float h = hsv[0], s = hsv[1], v = hsv[2], p, q, t, f, r, g, b;

	(h == 360.) ? (h = 0.) : (h /= 60.);
	f = h - floorf(h);

	p = v*(1.f - s);
	q = v*(1.f - s*f);
	t = v*(1.f - s*(1.f - f));

	if (0. <= h && h < 1.){
		r = v; g = t; b = p;
	}	else if (1. <= h && h < 2.) {
		r = q; g = v; b = p;
	}	else if (2. <= h && h < 3.){
		r = p; g = v; b = t;
	}else if (3. <= h && h < 4.){
		r = p; g = q; b = v;
	}else if (4. <= h && h < 5.){
		r = t; g = p; b = v;
	}else if (5. <= h && h < 6.){
		r = v; g = p;b = q;
	}
	else{
		r = 0.; g = 0.; b = 0.;
	}
	rgb[0] = r; rgb[1] = g; rgb[2] = b;
}