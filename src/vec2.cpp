#pragma once
#include "vec2.h"
#include <algorithm>

vec2::vec2() {
	x = 0;
	y = 0;
	i = false;
}
vec2::vec2(double X, double Y) {
	x = X;
	y = Y;
	i = true;
}
vec2::vec2(const vec2& v) {
	x = v.x;
	y = v.y;
	i = v.i;
}
void vec2::swap() {
	std::swap(x, y);
}
void vec2::perpindiculate() {
	std::swap(x, y);
	y = -y;
}
void vec2::normalize() {
	*this /= magnitude();
}
void vec2::rotate(double radians, vec2 center) {
	vec2 ogPts = *this -= center;
	x = ogPts.x * cos(radians) - ogPts.y * sin(radians);
	y = ogPts.y * cos(radians) + ogPts.x * sin(radians);
	*this += center;
}
double vec2::angle() const {
	return atan2(y, x);
}
double vec2::magnitude() const {
	return sqrt(x * x + y * y);
}
double vec2::magnitudeSquared() const {
	return x * x + y * y;
}
bool vec2::initialized() const {
	return i;
}

vec2 vec2::operator+(const vec2& v) const {
	return vec2(x + v.x, y + v.y);
}
vec2 vec2::operator-(const vec2& v) const {
	return vec2(x - v.x, y - v.y);
}
vec2 vec2::operator*(const vec2& v) const {
	return vec2(x * v.x, y * v.y);
}
vec2 vec2::operator*(double v) const {
	return vec2(x * v, y * v);
}
vec2 vec2::operator/(double v) const {
	return vec2(x / v, y / v);
}
vec2 vec2::operator+=(const vec2& v) {
	x += v.x;
	y += v.y;
	return *this;
}
vec2 vec2::operator-=(const vec2& v) {
	x -= v.x;
	y -= v.y;
	return *this;
}
vec2 vec2::operator*=(const vec2& v) {
	x *= v.x;
	y *= v.y;
	return *this;
}
vec2 vec2::operator*=(double v) {
	x *= v;
	y *= v;
	return *this;
}
vec2 vec2::operator/=(double v) {
	x /= v;
	y /= v;
	return *this;
}
bool vec2::operator==(const vec2& v) const {
	if (!i) return false;
	return v.x == x && v.y == y;
}
bool vec2::operator!=(const vec2& v) const {
	if (!i) return false;
	return v.x != x || v.y != y;
}
vec2& vec2::operator=(const vec2& v) {
	x = v.x;
	y = v.y;
	i = v.i;
	return *this;
}

vec2 normalize(vec2 v) {
	v.normalize();
	return v;
}