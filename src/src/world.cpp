#include "world.h"

PHYSICS_BEG

void get_dv(const particle &p1, glm::vec2 p1_center,
			const particle &p2, glm::vec2 p2_center,
			glm::vec2 collision_pt, glm::vec2 normal, float e,
			glm::vec2 *dp1v, float *dp1w,
			glm::vec2 *dp2v, float *dp2w)
{
	// https://physics.stackexchange.com/questions/786641/collision-calculation-in-2d
	// thank you to John Alexiou
	float x1 = p1_center.x;
	float y1 = p1_center.y;
	float x2 = p2_center.x;
	float y2 = p2_center.y;
	float xa = collision_pt.x;
	float ya = collision_pt.y;
	float vx1 = p1.v.x;
	float vy1 = p1.v.y;
	float vx2 = p2.v.x;
	float vy2 = p2.v.y;
	float nx = normal.x;
	float ny = normal.y;
	float w1 = p1.w;
	float w2 = p2.w;
	float m1 = p1.m;
	float m2 = p2.m;
	float I1 = p1.I;
	float I2 = p2.I;

	float v_imp = nx * (vx1 + w1 * (y1 - ya) - vx2 - w2 * (y2 - ya)) +
				  ny * (vy1 + w1 * (xa - x1) - vy2 - w2 * (xa - x2));
	
	float s1 = nx * (y1 - ya) + ny * (xa - x1);
	float s2 = nx * (y2 - ya) + ny * (xa - x2);
	float m_imp = 1.f /
					((1 / m1) + (s1 * s1) / I1 + 1 / m2 + (s2 * s2) / I2);
	
	float J = (1 + e) * m_imp * v_imp;

	if (dp1v)
	{
		float dvx1 = -nx / m1 * J;
		float dvy1 = -ny / m1 * J;

		dp1v->x += dvx1;
		dp1v->y += dvy1;
	}

	if (dp1w)
	{
		float dw1 = -s1 / I1 * J;

		*dp1w += dw1;
	}

	if (dp2v)
	{
		float dvx2 = nx / m2 * J;
		float dvy2 = ny / m2 * J;
			
		dp2v->x += dvx2;
		dp2v->y += dvy2;
	}

	if (dp2w)
	{
		float dw2 = s2 / I2 * J;
		*dp2w += dw2;
	}
}

void resolve_velocities(particle &p1, glm::vec2 p1_center, particle &p2, glm::vec2 p2_center, const collision &coll, float e)
{
	glm::vec2 dp1v(0, 0);
	glm::vec2 dp2v(0, 0);
	float dp1w = 0.f;
	float dp2w = 0.f;
	
	// if (collision_pt.size() == 1)
	// {
	// 	get_dv(p1, p1_center, p2, p2_center, *collision_pt.pts[0], coll.normal, e, &dp1v, &dp1w, &dp2v, &dp2w);
	// }
	// else if (collision_pt.size() == 2)
	// {
		// get the change in angular velocity without the change in linear velocity
		// get_dv(p1, p1_center, p2, p2_center, coll.a_contact, coll.normal, e, nullptr, &dp1w, nullptr, &dp2w);
		// get_dv(p1, p1_center, p2, p2_center, coll.b_contact, coll.normal, e, nullptr, &dp1w, nullptr, &dp2w);

		// get the change in linear velocity without the change in angular velocity
		// glm::vec2 average = (coll.a_contact + coll.b_contact) / 2.f;
		// get_dv(p1, p1_center, p2, p2_center, average, coll.normal, e, &dp1v, nullptr, &dp2v, nullptr);
	// }
	// else
	// 	return;

	p1.v += dp1v;
	p2.v += dp2v;

	p1.w += dp1w;
	p2.w += dp2w;
}

world::world(float world_width_meters, float world_height_meters, float gravity) : grav{ gravity }, world_width{ world_width_meters }, world_height{ world_height_meters }
{
	static polygon<4> rect = {glm::vec2{0, 0}, {1, 0}, {1, 1}, {0, 1}};
	
	constexpr float bound_width = 10'000'000.f;
	// bottom wall
	objects.push_back({{{-bound_width, -bound_width}, {0, 0}, {0, 0}, 0, 0, 0, particle::infinity, particle::infinity}, {bound_width * 2 + world_width, bound_width}, &rect});
	// left wall
	objects.push_back({{{-bound_width, 0}, {0, 0}, {0, 0}, 0, 0, 0, particle::infinity, particle::infinity}, {bound_width, world_height}, &rect});
	// right wall
	objects.push_back({{{world_width, 0}, {0, 0}, {0, 0}, 0, 0, 0, particle::infinity, particle::infinity}, {bound_width, world_height}, &rect});
	// top wall
	objects.push_back({{{-bound_width, world_height}, {0, 0}, {0, 0}, 0, 0, 0, particle::infinity, particle::infinity}, {bound_width * 2 + world_width, bound_width}, &rect});
}

template <typename object>
bool object_compare(const object &a, const object &b)
{
	return a.pt.pos.y > b.pt.pos.y;
}

object *world::add_object(const abstract_shape &shape, glm::vec2 pos, glm::vec2 v_init, float angle, float w_init, float mass, glm::vec2 scale)
{
	objects.push_back({{pos, v_init, {0, grav}, angle, w_init, 0, mass, mass * 10}, scale, &shape});
	auto res = &objects.back();
	objects.sort(object_compare<object>); // temporary solution

	return res;
}

object *world::add_static_object(const abstract_shape &shape, glm::vec2 pos, float angle, glm::vec2 scale)
{
	objects.push_back({{pos, {0, 0}, {0, 0}, angle, 0, 0, particle::infinity, particle::infinity}, scale, &shape});
	auto res = &objects.back();
	objects.sort(object_compare<object>); // temporary solution

	return res;
}

void world::update_internal()
{
	for (auto &obj : objects)
		obj.pt.update(time_step);

	resolve_bounds();

	for (const auto &c : constraints)
		c->update(time_step);
	
	for (const auto &[a, b, coll] : collisions)
	{
		glm::vec2 a_center = a->shape->center() * a->scale + a->pt.pos;
		glm::vec2 b_center = b->shape->center() * b->scale + b->pt.pos;

		resolve_velocities(a->pt, a_center, b->pt, b_center, coll, .85f);
	}
}

void world::resolve_bounds()
{
	constexpr float epsilon = 1E-6f;

	collisions.clear();

	objects.sort(object_compare<object>); // temporary solution
	
	auto a_end = std::prev(objects.end());
	for (auto a = objects.begin(); a != a_end; ++a)
	{
		for (auto b = std::next(a); b != objects.end(); ++b)
		{
			shape_view a_view(*a->shape, a->pt.pos, a->scale, a->pt.angle);
			shape_view b_view(*b->shape, b->pt.pos, b->scale, b->pt.angle);

			auto res = collides(a_view, b_view);
			if (!res)
				continue;
			auto mtv = res.normal * res.dist;
			if (std::abs(mtv.x) < epsilon && std::abs(mtv.y) < epsilon)
				continue;

			collisions.push_back({a, b, res});
			
			bool a_inf = a->pt.m == particle::infinity;
			// bool b_inf = b->pt.m == particle::infinity;

			if (a_inf)
				b->pt.pos -= mtv;
			// else if (b_inf)
			// 	a->pt.pos += mtv;
			else
				a->pt.pos += mtv;
		}
	}
}

PHYSICS_END