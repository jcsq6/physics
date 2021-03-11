#pragma once
#define _USE_MATH_DEFINES
#include <cmath>
#include <algorithm>

template <class T> class vec2 {
public:
	vec2() {
		x = 0;
		y = 0;
		i = false;
	}
	vec2(T X, T Y) {
		x = X;
		y = Y;
		i = true;
	}
	vec2(const vec2 &v) {
		x = v.x;
		y = v.y;
		i = v.i;
	}
	operator vec2<T&>() {
		return vec2<T&>(x, y);
	}
	void swap() {
		std::swap(x, y);
	}
	void perpindiculate() {
		std::swap(x, y);
		y = -y;
	}
	void normalize() {
		*this /= magnitude();
	}
	void rotate(double radians, vec2 center) {
		vec2 ogPts = *this -= center;
		x = ogPts.x * cos(radians) - ogPts.y * sin(radians);
		y = ogPts.y * cos(radians) + ogPts.x * sin(radians);
		*this += center;
	}
	double angle() const {
		return atan2(y, x);
	}
	double magnitude() const {
		return sqrt(x * x + y * y);
	}
	double magnitudeSquared() const {
		return x * x + y * y;
	}
	vec2 operator+(const vec2& v) const {
		return vec2(x + v.x, y + v.y);
	}
	vec2 operator-(const vec2& v) const {
		return vec2(x - v.x, y - v.y);
	}
	vec2 operator*(const vec2& v) const {
		return vec2(x * v.x, y * v.y);
	}
	vec2 operator*(T v) const {
		return vec2(x * v, y * v);
	}
	vec2 operator/(T v) const {
		return vec2(x / v, y / v);
	}
	vec2 operator+=(const vec2& v) {
		x += v.x;
		y += v.y;
		return *this;
	}
	vec2 operator-=(const vec2& v) {
		x -= v.x;
		y -= v.y;
		return *this;
	}
	vec2 operator*=(const vec2& v) {
		x *= v.x;
		y *= v.y;
		return *this;
	}
	vec2 operator*=(T v) {
		x *= v;
		y *= v;
		return *this;
	}
	vec2 operator/=(T v) {
		x /= v;
		y /= v;
		return *this;
	}
	bool operator==(const vec2 &v) const {
		if (!i) return false;
		return v.x == x && v.y == y;
	}
	bool operator!=(const vec2 &v) const {
		if (!i) return false;
		return v.x != x || v.y != y;
	}
	vec2 &operator=(const vec2 &v) {
		x = v.x;
		y = v.y;
		i = v.i;
		return *this;
	}
	bool initialized() {
		return i;
	}
	T x;
	T y;
private:
	bool i;
};

template<class T>
class vec2<T&> {
public:
	vec2(T& X, T& Y) : x{ X }, y{ Y } {}
	vec2(const vec2<T&>&) = delete;
	vec2(vec2<T>&&) = delete;
	operator vec2<T>() {
		return vec2<T>(x, y);
	}
	vec2<T> swap() const {
		return vec2<T>(y, x);
	}
	vec2<T> perpindiculate() const {
		return vec2<T>(-y, x);
	}
	vec2<T> normalize() const {
		return *this / magnitude();
	}
	vec2<T> rotate(double radians, vec2 center) const {
		return vec2<T>(x * cos(radians) - y * sin(radians), y * cos(radians) + x * sin(radians));
	}
	double angle() const {
		return atan((float)y / (float)x);
	}
	double magnitude() const {
		return sqrt(x * x + y * y);
	}
	double magnitudeSquared() const {
		return x * x + y * y;
	}
	vec2 operator+(const vec2& v) const {
		return vec2(x + v.x, y + v.y);
	}
	vec2 operator-(const vec2& v) const {
		return vec2(x - v.x, y - v.y);
	}
	vec2 operator*(const vec2& v) const {
		return vec2(x * v.x, y * v.y);
	}
	vec2 operator*(T& v) const {
		return vec2(x * v, y * v);
	}
	vec2 operator/(T& v) const {
		return vec2(x / v, y / v);
	}
	bool operator==(const vec2& v) const {
		return (v.x == x && v.y == y);
	}
	bool operator!=(const vec2& v) const {
		return (v.x != x || v.y != y);
	}
	vec2<T&> operator=(vec2<T&>) = delete;
	vec2<T&> operator=(vec2<T&>&&) = delete;
	T getx() {
		return x;
	}
	T gety() {
		return y;
	}
private:
	T& x;
	T& y;
};

template <class T> int sgn(T val) {
	return (T(0) < val) - (val < T(0));
}

template <class T> inline bool are_parallel(vec2<T> v1, vec2<T> v2) {
	if (v1.x / v2.x == v1.y / v2.y) return true;
}
template <class T> inline double det(vec2<T> v1, vec2<T> v2) {
	return v1.x * v2.y - v2.x * v1.y;
}
template <class T> inline double cross(vec2<T> a, vec2<T> b) {
	return a.x * b.y - a.y * b.x;
}
template <class T> inline double dot(vec2<T> a, vec2<T> b) {
	return a.x * b.x + a.y * b.y;
}
template <class T> inline double distance(vec2<T> a, vec2<T> b) {
	return sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}
template <class T> inline double distanceSquared(vec2<T> a, vec2<T> b) {
	return (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y);
}
template <class T> inline bool isInRadius(vec2<T> p1, vec2<T> p2, double r) {
	return distanceSquared(p1, p2) <= r * r;
}