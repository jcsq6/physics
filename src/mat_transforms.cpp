#include "mat.h"

mat<double> translate(double x, double y) {
	mat<double> temp = {
		{1.0, 0.0, x  },
		{0.0, 1.0, y  },
		{0.0, 0.0, 1.0},
	};
	return temp;
}

mat<double> scale(double x_scale, double y_scale) {
	mat<double> temp = {
		{x_scale, 0.0, 0.0},
		{0.0, y_scale, 0.0},
		{0.0, 0.0,     1.0},
	};
	return temp;
}

mat<double> rotate(double radians) {
	mat<double> temp = {
		{cos(radians), -sin(radians), 0.0},
		{sin(radians),  cos(radians), 0.0},
		{0.0,           0.0,          1.0},
	};
	return temp;
}

mat<double> shear(double x_shear, double y_shear) {
	mat<double> temp = {
		{1.0, x_shear, 0.0},
		{y_shear, 1.0, 0.0},
		{0.0,     0.0, 1.0},
	};
	return temp;
}

mat<double> reflect(const vec2& l) {
	mat<double> temp = {
		{l.x * l.x - l.y * l.y, 2 * l.x * l.y, 0.0},
		{2 * l.x * l.y, l.y * l.y - l.x * l.x, 0.0},
		{0.0,           0.0,                   1.0},
	};
	return temp * (1 / l.magnitude());
}