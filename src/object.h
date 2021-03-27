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

	void addVelocity(vec2 velocity);
	void setVelocity(vec2 velocity);
	void addImpulse(vec2 force, double duration);
	void addForce(vec2 force);
	void setForce(vec2 force);
	void addConstAccel(vec2 accel, std::string name);
	void removeConstAcceleration(std::string name);

	vec2 getConstAcceleration(std::string name);
	bool hasConstAcceleration(std::string name) const;
	int numOfConstAccelerations() const;
	vec2 getUpcomingNetForce() const;
	vec2 getPastNetForce() const;
	vec2 getVel() const;
	vec2 getAccel() const;
	double getMassRecip() const;
	double getMass() const;
	double getRestitution();

	object& operator=(const object& o) noexcept;
	object& operator=(object&& o) noexcept;

	poly bounds;
private:
	double m_recip;
	double cor;
	vec2 vel;
	vec2 accel;
	vec2 netForce;
	vec2 netForceBack;
	std::map<std::string, vec2> constAcceleration;
	std::vector<std::pair<vec2, double>> impulses;
};