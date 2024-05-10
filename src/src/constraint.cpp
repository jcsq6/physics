#include "constraint.h"

PHYSICS_BEG

void position_constraint::update(float dt)
{
	glm::vec3 rel_pos = a->pt.pos - b->pt.pos;
	float cur_dist = glm::length(rel_pos);
	float delta = dist - cur_dist;
	if (delta != 0)
	{
		glm::vec3 dir = glm::normalize(rel_pos);
		glm::vec3 rel_vel = a->pt.vel - b->pt.vel;

		float mass = a->pt.mass + b->pt.mass;
		float proj = glm::dot(rel_vel, dir);

		float bias = -constraint::factor / dt * delta;
		float lagrange = -(proj + bias) * mass;

		a->pt.v += dir * lagrange / a->pt.mass;
		b->pt.v -= dir * lagrange / b->pt.mass;
	}
}

PHYSICS_END