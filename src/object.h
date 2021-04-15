#pragma once
#include "poly.h"
#include <ostream>
#include <string>
#include <map>

class object {
public:
	object(double mass, double cor, const poly& bounds, const vec2& init_vel = { 0, 0 }, int R = 0, int G = 0, int B = 0, int A = 255);
	object(double mass, double cor, poly&& bounds, const vec2& init_vel = { 0, 0 }, int R = 0, int G = 0, int B = 0, int A = 255);
	object(const object& o) noexcept;
	object(object&& o) noexcept;

	void update(double dt);

	void add_const_accel(const vec2& accel, std::string name);
	void remove_const_accel(std::string name);

	vec2 get_const_accel(std::string name);
	bool has_const_accel(std::string name) const;
	int num_of_const_accels() const;

	vec2 get_prev_net_force() const;
	vec2 get_prev_accel() const;

	object& operator=(const object& o) noexcept;
	object& operator=(object&& o) noexcept;

	poly bounds;
	double m_recip;
	double restitution;
	vec2 vel;
	vec2 net_force;
	vec2 accel;
	double angular_vel;
	int r, g, b, a;
private:
	vec2 net_force_back;
	vec2 accel_back;
	vec2 net_const_accel;
	std::map<std::string, vec2> const_accel;
};

std::ostream& operator<<(std::ostream& o, const object& obj);