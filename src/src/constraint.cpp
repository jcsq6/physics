#include "constraint.h"

PHYSICS_BEG

void position_constraint::update(float dt)
{
	glm::vec2 rel_pos = a->pt.pos - b->pt.pos;
	float cur_dist = glm::length(rel_pos);
	float delta = dist - cur_dist;
	if (delta != 0)
	{
		glm::vec2 dir = glm::normalize(rel_pos);
		glm::vec2 rel_vel = a->pt.v - b->pt.v;

		float inv_mass = 1 / a->pt.m + 1 / b->pt.m;
		if (inv_mass <= 0)
			return;

		float proj = glm::dot(rel_vel, dir);

		float bias = -constraint::factor / dt * delta;
		float lagrange = -(proj + bias) / inv_mass;

		a->pt.v += dir * lagrange / a->pt.m;
		b->pt.v -= dir * lagrange / b->pt.m;
	}
}

PHYSICS_END