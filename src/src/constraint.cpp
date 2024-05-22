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

void rope_constraint::update(float dt)
{
	glm::vec2 rel_pos = a->pt.pos - b->pt.pos;
	float cur_dist = glm::length(rel_pos);
	float delta = dist - cur_dist;
	if (delta < 0)
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

void collision_constraint::update(float dt)
{
	// shape_view a_view(*a->poly, a->pt.pos, a->scale, a->pt.angle);
	// shape_view b_view(*b->poly, b->pt.pos, b->scale, b->pt.angle);

	// auto res = collides(a_view, b_view);
	// if (!res || (std::abs(res.mtv.x) < epsilon && std::abs(res.mtv.y) < epsilon))
	// 	continue;

	// auto manifold = contact_manifold(a_view, b_view, res);

	// Eigen::Vector<float, 6> v(a->pt.v.x, a->pt.v.y, b->pt.v.x, b->pt.v.y, a->pt.w, b->pt.w);
}

PHYSICS_END