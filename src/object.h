#pragma once
#include "poly.h"
#include <string>
#include <map>

class object {
public:
	object(double mass, double restitution, poly boundingBox);
	object(const object& o) noexcept;
	object(object&& o) noexcept;

	void update(double dt);

	void addVelocity(vec2<double> velocity);
	void setVelocity(vec2<double> velocity);
	void addImpulse(vec2<double> force, double duration);
	void addForce(vec2<double> force);
	void setForce(vec2<double> force);
	void addConstAccel(vec2<double> accel, std::string name);
	void removeConstAcceleration(std::string name);

	vec2<double> getConstAcceleration(std::string name);
	bool hasConstAcceleration(std::string name) const;
	int numOfConstAccelerations() const;
	vec2<double> getUpcomingNetForce() const;
	vec2<double> getPastNetForce() const;
	vec2<double> getVel() const;
	vec2<double> getAccel() const;
	double getMassRecip() const;
	double getMass() const;
	double getRestitution();

	object& operator=(const object& o) noexcept;
	object& operator=(object&& o) noexcept;

	poly bounds;
private:
	double m_recip;
	double cor;
	vec2<double> vel;
	vec2<double> accel;
	vec2<double> netForce;
	vec2<double> netForceBack;
	std::map<std::string, vec2<double>> constAcceleration;
	std::vector<std::pair<vec2<double>, double>> impulses;
};