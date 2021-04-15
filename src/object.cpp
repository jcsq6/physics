#include "object.h"

object::object(double mass, double cor, const poly& bounds, const vec2& init_vel, int R, int G, int B, int A) :
	bounds{ bounds },
	vel{ init_vel }
{
	restitution = cor;
	m_recip = 1 / mass;
	angular_vel = 0;
	r = R;
	g = G;
	b = B;
	a = A;
}
object::object(double mass, double cor, poly&& bounds, const vec2& init_vel, int R, int G, int B, int A) :
	bounds{ std::move(bounds) },
	vel{ init_vel }
{
	restitution = cor;
	m_recip = 1 / mass;
	angular_vel = 0;
	r = R;
	g = G;
	b = B;
	a = A;
}
object::object(const object& o) noexcept :
	bounds{ o.bounds },
	vel{ o.vel },
	net_const_accel{ o.net_const_accel },
	const_accel{ o.const_accel },
	net_force{ o.net_force },
	net_force_back{ o.net_force_back },
	accel{ o.accel },
	accel_back{ o.accel_back }
{
	restitution = o.restitution;
	m_recip = o.m_recip;
	angular_vel = o.angular_vel;
	r = o.r;
	g = o.g;
	b = o.b;
	a = o.a;
}
object::object(object&& o) noexcept :
	bounds{ std::move(o.bounds) },
	vel{ o.vel },
	net_const_accel{ o.net_const_accel },
	const_accel{ std::move(o.const_accel) },
	net_force{ o.net_force },
	net_force_back{ o.net_force_back },
	accel{ o.accel },
	accel_back{ o.accel_back }
{
	restitution = o.restitution;
	m_recip = o.m_recip;
	angular_vel = o.angular_vel;
	r = o.r;
	g = o.g;
	b = o.b;
	a = o.a;
}
void object::update(double dt) {
	accel += net_const_accel + net_force * m_recip;
	vel += accel * dt;
	bounds += vel * dt;
	net_force_back = accel / m_recip;
	accel_back = accel;
	accel.x = accel.y = 0;
	net_force.x = net_force.y = 0;
}

void object::add_const_accel(const vec2& accel, std::string name) {
	const_accel[name] = accel;
	net_const_accel += accel;
}
void object::remove_const_accel(std::string name) {
	if (auto it = const_accel.find(name); it != const_accel.end()) {
		net_const_accel -= it->second;
		const_accel.erase(it);
	}
}
vec2 object::get_const_accel(std::string name) {
	return const_accel.at(name);
}
bool object::has_const_accel(std::string name) const {
	return const_accel.find(name) != const_accel.end();
}
int object::num_of_const_accels() const {
	return const_accel.size();
}

vec2 object::get_prev_net_force() const {
	return net_force_back;
}

vec2 object::get_prev_accel() const {
	return accel_back;
}

object &object::operator=(const object& o) noexcept {
	bounds = o.bounds;
	const_accel = o.const_accel;
	net_const_accel = o.net_const_accel;
	accel = o.accel;
	accel_back = o.accel_back;
	net_force = o.net_force;
	net_force_back = o.net_force_back;
	vel = o.vel;
	restitution = o.restitution;
	m_recip = o.m_recip;
	angular_vel = o.angular_vel;
	r = o.r;
	g = o.g;
	b = o.b;
	a = o.a;
	return *this;
}
object &object::operator=(object&& o) noexcept {
	bounds = std::move(o.bounds);
	const_accel = std::move(o.const_accel);
	net_const_accel = o.net_const_accel;
	accel = o.accel;
	accel_back = o.accel_back;
	net_force = o.net_force;
	net_force_back = o.net_force_back;
	vel = o.vel;
	restitution = o.restitution;
	m_recip = o.m_recip;
	angular_vel = o.angular_vel;
	r = o.r;
	g = o.g;
	b = o.b;
	a = o.a;
	return *this;
}

std::ostream& operator<<(std::ostream& o, const object& obj){
	o << "accel: " << obj.accel.x << ", " << obj.accel.y << std::endl;
	o << "vel: " << obj.vel.x << ", " << obj.vel .y << std::endl;
	return o << "pos: " << obj.bounds.begin()->x << ", " << obj.bounds.begin()->y;
}