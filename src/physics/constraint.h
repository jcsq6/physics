#ifndef CONSTRAINT_H
#define CONSTRAINT_H

#include <Eigen/Dense>
#include <vector>

#include "object.h"

PHYSICS_BEG

class constraint
{
public:
	static constexpr float factor = .01f;
	virtual void update(float dt) = 0;
	virtual ~constraint() = default;
};

class position_constraint : public constraint
{
public:
	position_constraint(object &object_a, object &object_b, float distance) : a{&object_a}, b{&object_b}, dist{distance}
	{
	}

	void update(float dt) override;

private:
	object *a, *b;
	float dist;
};

// constraint that keeps two objects <= a certain distance from each other
class rope_constraint : public constraint
{
public:
	rope_constraint(object &object_a, object &object_b, float distance) : a{&object_a}, b{&object_b}, dist{distance}
	{
	}

	void update(float dt) override;

private:
	object *a, *b;
	float dist;
};

class collision_constraint : public constraint
{
public:
	collision_constraint(object &object_a, object &object_b) : a{&object_a}, b{&object_b}
	{
	}

	void update(float dt) override;
private:
	object *a, *b;
};

PHYSICS_END

#endif