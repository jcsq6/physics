#ifndef CONSTRAINT_H
#define CONSTRAINT_H

#include <Eigen/Dense>
#include <vector>

#include "object.h"

PHYSICS_BEG

class constraint
{
public:
	static constexpr float factor = .01;
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

PHYSICS_END

#endif