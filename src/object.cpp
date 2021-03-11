#include "object.h"

object::object(double mass, double restitution, poly boundingBox) :
	bounds{ boundingBox }, vel{ vec2<double>(0,0) }, accel{ vec2<double>(0,0) },
	netForce{ vec2<double>(0, 0) }, netForceBack{ vec2<double>(0, 0) }{
	cor = restitution;
	m_recip = 1 / mass;
}
object::object(const object& o) noexcept : impulses{ o.impulses }, constAcceleration{ o.constAcceleration }, bounds{ o.bounds }{
	cor = o.cor;
	m_recip = o.m_recip;
	vel = o.vel;
	accel = o.accel;
	netForce = o.netForce;
	netForceBack = o.netForceBack;
}
object::object(object&& o) noexcept : constAcceleration{ std::move(o.constAcceleration) }, impulses{ std::move(o.impulses) }, bounds{ std::move(o.bounds) }{
	cor = o.cor;
	m_recip = o.m_recip;
	vel = o.vel;
	accel = o.accel;
	netForce = o.netForce;
	netForceBack = o.netForceBack;
}
void object::update(double dt) {
	for (int i = 0; i < impulses.size(); i++) {
		impulses[i].second -= dt;
		netForce += impulses[i].first;
		if (impulses[i].second <= 0) impulses.erase(impulses.begin() + i);
	}
	accel = netForce * m_recip;
	for (const auto& a : constAcceleration) {
		accel += a.second;
	}
	vel += accel * dt;
	bounds += vel * dt;
	netForceBack = netForce;
	netForce.x = 0;
	netForce.y = 0;
}
void object::addVelocity(vec2<double> velocity) {
	vel += velocity;
}
void object::setVelocity(vec2<double> velocity) {
	vel = velocity;
}
void object::addImpulse(vec2<double> force, double duration) {
	impulses.push_back(std::make_pair(force, duration));
}
void object::addForce(vec2<double> force) {
	netForce += force;
}
void object::setForce(vec2<double> force) {
	netForce = force;
}
void object::addConstAccel(vec2<double> accel, std::string name) {
	constAcceleration[name] = accel;
}
void object::removeConstAcceleration(std::string name) {
	constAcceleration.erase(name);
}
vec2<double> object::getConstAcceleration(std::string name) {
	return constAcceleration[name];
}
bool object::hasConstAcceleration(std::string name) const {
	return constAcceleration.count(name);
}
int object::numOfConstAccelerations() const {
	return constAcceleration.size();
}
vec2<double> object::getUpcomingNetForce() const {
	return netForce;
}
vec2<double> object::getPastNetForce() const {
	return netForceBack;
}
vec2<double> object::getVel() const {
	return vel;
}
vec2<double> object::getAccel() const {
	return accel;
}
double object::getMassRecip() const {
	return m_recip;
}
double object::getMass() const {
	return 1 / m_recip;
}
double object::getRestitution() {
	return cor;
}
object &object::operator=(const object& o) noexcept {
	cor = o.cor;
	m_recip = o.m_recip;
	vel = o.vel;
	accel = o.accel;
	netForce = o.netForce;
	netForceBack = o.netForceBack;
	bounds = o.bounds;
	constAcceleration = o.constAcceleration;
	impulses = o.impulses;
	return *this;
}
object &object::operator=(object&& o) noexcept {
	cor = o.cor;
	m_recip = o.m_recip;
	vel = o.vel;
	accel = o.accel;
	netForce = o.netForce;
	netForceBack = o.netForceBack;
	bounds = std::move(o.bounds);
	constAcceleration = std::move(o.constAcceleration);
	impulses = std::move(o.impulses);
	return *this;
}