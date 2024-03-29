#include "world.h"

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

void resolve_velocities(particle &p1, glm::vec2 p1_center, particle &p2, glm::vec2 p2_center, manifold collision_pt, glm::vec2 normal, float e)
{
	glm::vec2 dp1v(0, 0);
	glm::vec2 dp2v(0, 0);
	float dp1w = 0.f;
	float dp2w = 0.f;
	
	if (collision_pt.size() == 1)
	{
		get_dv(p1, p1_center, p2, p2_center, *collision_pt.pts[0], normal, e, &dp1v, &dp1w, &dp2v, &dp2w);
	}
	else if (collision_pt.size() == 2)
	{
		// get the change in angular velocity without the change in linear velocity
		get_dv(p1, p1_center, p2, p2_center, *collision_pt.pts[0], normal, e, nullptr, &dp1w, nullptr, &dp2w);
		get_dv(p1, p1_center, p2, p2_center, *collision_pt.pts[1], normal, e, nullptr, &dp1w, nullptr, &dp2w);

		// get the change in linear velocity without the change in angular velocity
		glm::vec2 average = (*collision_pt.pts[0] + *collision_pt.pts[1]) / 2.f;
		get_dv(p1, p1_center, p2, p2_center, average, normal, e, &dp1v, nullptr, &dp2v, nullptr);
	}
	else
		return;

	p1.v += dp1v;
	p2.v += dp2v;

	p1.w += dp1w;
	p2.w += dp2w;
}

template <typename It, typename Comp>
void insertion_sort(It begin, It end, Comp cmp)
{
	for (auto it = std::next(begin); it != end; ++it)
	{
		auto key = *it;
		auto j = it;
		while (j != begin && cmp(key, *std::prev(j)))
		{
			*j = *std::prev(j);
			--j;
		}

		*j = std::move(key);
	}
}

world::world(float world_width_meters, float world_height_meters, float gravity) : grav{ gravity }, world_width{ world_width_meters }, world_height{ world_height_meters }
{
	static polygon rect = []{
		polygon res;
		res.reserve(4);
		res.push_back({0, 0});
		res.push_back({1, 0});
		res.push_back({1, 1});
		res.push_back({0, 1});
		return res;
	}();

	auto it = shapes.emplace(&rect, std::unique_ptr<draw_poly>{}).first;
	
	constexpr float bound_width = 10'000'000.f;
	// bottom wall
	objects.push_back({{{-bound_width, -bound_width}, {0, 0}, {0, 0}, 0, 0, 0, particle::infinity, particle::infinity}, {}, {bound_width * 2 + world_width, bound_width}, it});
	// left wall
	objects.push_back({{{-bound_width, 0}, {0, 0}, {0, 0}, 0, 0, 0, particle::infinity, particle::infinity}, {}, {bound_width, world_height}, it});
	// right wall
	objects.push_back({{{world_width, 0}, {0, 0}, {0, 0}, 0, 0, 0, particle::infinity, particle::infinity}, {}, {bound_width, world_height}, it});
	// top wall
	objects.push_back({{{-bound_width, world_height}, {0, 0}, {0, 0}, 0, 0, 0, particle::infinity, particle::infinity}, {}, {bound_width * 2 + world_width, bound_width}, it});
}

bool object_compare(const object &a, const object &b)
{
	return a.pt.pos.y > b.pt.pos.y;
}

void world::add_object(const polygon &poly, glm::vec2 pos, glm::vec2 v_init, float angle, float w_init, float mass, glm::vec2 scale, const glm::vec4 &color)
{
	auto it = shapes.find(&poly);
	if (it == shapes.end())
		it = shapes.emplace(&poly, std::make_unique<draw_poly>(poly.pts_begin(), poly.pts_end())).first;
	objects.push_back({{pos, v_init, {0, grav}, angle, w_init, 0, mass, mass * 10}, color, scale, it});
	std::sort(objects.begin(), objects.end(), object_compare);
}

void world::update_internal()
{
	for (auto &obj : objects)
		obj.pt.update(time_step);

	resolve_bounds();
	
	for (const auto &[a, b, normal, contact_pts] : collisions)
	{
		// auto old_energy = (.5f * a->pt.m * a->pt.v.x * a->pt.v.x) + (.5f * a->pt.m * a->pt.v.y * a->pt.v.y) +
		// 				  (.5f * b->pt.m * b->pt.v.x * b->pt.v.x) + (.5f * b->pt.m * b->pt.v.y * b->pt.v.y) +
		// 				  (.5 * a->pt.I * a->pt.w * a->pt.w) + (.5 * b->pt.I * b->pt.w * b->pt.w);

		glm::vec2 a_center = a->poly->first->center() * a->scale + a->pt.pos;
		glm::vec2 b_center = b->poly->first->center() * b->scale + b->pt.pos;

		resolve_velocities(a->pt, a_center, b->pt, b_center, contact_pts, normal, .9f);
	}
}

void world::resolve_bounds()
{
	constexpr float epsilon = 1E-6f;

	collisions.clear();

	insertion_sort(objects.begin(), objects.end(), object_compare);
	
	for (auto a = objects.begin(); a < objects.end() - 1; ++a)
	{
		for (auto b = a + 1; b < objects.end(); ++b)
		{
			polygon_view a_view(*a->poly->first, a->pt.pos, a->scale, a->pt.angle);
			polygon_view b_view(*b->poly->first, b->pt.pos, b->scale, b->pt.angle);

			auto res = collides(a_view, b_view);
			if (!res || (std::abs(res.mtv.x) < epsilon && std::abs(res.mtv.y) < epsilon))
				continue;

			collisions.push_back({a, b, res.normal, contact_manifold(a_view, b_view, res)});
			
			bool a_inf = a->pt.m == particle::infinity;
			bool b_inf = b->pt.m == particle::infinity;

			if (a_inf)
			{
				b->pt.pos -= res.mtv;
			}
			else if (b_inf)
			{
				a->pt.pos += res.mtv;
			}
			else
			{
				a->pt.pos += res.mtv;
			}
		}
	}
}