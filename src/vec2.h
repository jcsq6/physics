#pragma once
#define _USE_MATH_DEFINES
#include <cmath>

template<typename C>
class mat;

class vec2 {
public:
	vec2();
	vec2(double X, double Y);
	vec2(const vec2& v);
	template<typename C>
	vec2(const mat<C>& m);

	void swap();
	void perpindiculate();
	void normalize();
	void rotate(double radians, vec2 center);
	double angle() const;
	double magnitude() const;
	double magnitudeSquared() const;

	vec2 operator+(const vec2& v) const;
	vec2 operator-(const vec2& v) const;
	vec2 operator*(double v) const;
	template<typename C>
	auto operator*(const mat<C>& m) const;
	vec2 operator/(double v) const;
	vec2& operator+=(const vec2& v);
	vec2& operator-=(const vec2& v);
	vec2& operator*=(double v);
	template<typename C>
	auto& operator*=(const mat<C>& m);
	vec2& operator/=(double v);
	bool operator==(const vec2& v) const;
	bool operator!=(const vec2& v) const;
	vec2& operator=(const vec2& v);

	double x;
	double y;
};

vec2 operator-(const vec2& v);

vec2 normalize(vec2 v);
vec2 perpindiculate(vec2 v);

template <class T> inline int sgn(T val) {
	return (T(0) < val) - (val < T(0));
}

inline double to_radians(double degrees) {
	return degrees * M_PI / 180;
}
inline double to_degrees(double radians) {
	return radians * 180 / M_PI;
}

inline bool are_parallel(const vec2& v1, const vec2& v2) {
	return v1.x * v2.y == v2.x * v1.y;
}
inline double det(const vec2& v1, const vec2& v2) {
	return v1.x * v2.y - v2.x * v1.y;
}
inline double cross(const vec2& a, const vec2& b) {
	return a.x * b.y - a.y * b.x;
}
inline double dot(const vec2& a, const vec2& b) {
	return a.x * b.x + a.y * b.y;
}
inline double distance(const vec2& a, const vec2& b) {
	return sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}
inline double distanceSquared(const vec2& a, const vec2& b) {
	return (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y);
}
inline bool isInRadius(const vec2& p1, const vec2& p2, double r) {
	return distanceSquared(p1, p2) <= r * r;
}
inline double angle(const vec2& v1, const vec2& v2) {
	return acos(dot(v1, v2) / (v1.magnitude() * v2.magnitude()));
}