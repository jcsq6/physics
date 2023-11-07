#include "bound.h"
#include <iostream>

bool project_onto(const polygon_view &a, const polygon_view &b, float &min_intersection, collision &res)
{
	for (auto a_edge = a.poly->pts_begin(); a_edge != a.poly->pts_end(); ++a_edge)
	{
		glm::vec2 normal = rotate(a_edge->normal, a.angle);

		float amin = std::numeric_limits<float>::infinity();
		float amax = -std::numeric_limits<float>::infinity();
		for (std::size_t i = 0; i < a.size(); ++i)
		{
			float cur = glm::dot(normal, a.point(i));
			if (cur < amin)
				amin = cur;
			if (cur > amax)
				amax = cur;
		}

		float bmin = std::numeric_limits<float>::infinity();
		float bmax = -std::numeric_limits<float>::infinity();
		for (std::size_t i = 0; i < b.size(); ++i)
		{
			float cur = glm::dot(normal, b.point(i));
			if (cur < bmin)
				bmin = cur;
			if (cur > bmax)
				bmax = cur;
		}

		if (amax < bmin || bmax < amin)
			return false;

		float cur_intersection = std::min(amax - bmin, bmax - amin);
		
		// if this intersection is less than the minimum
		if (cur_intersection < min_intersection)
		{
			min_intersection = cur_intersection;
			res.mtv = normal * min_intersection;
			res.normal = normal;
		}
	}

	return true;
}

// returns collision with mtv to get a out of b or false if no collision
collision collides(const polygon_view &a, const polygon_view &b)
{
	collision res;
	res.collides = true;
	float intersection = std::numeric_limits<float>::infinity();
	if (project_onto(a, b, intersection, res))
	{
		if (project_onto(b, a, intersection, res))
		{
			glm::vec2 center_diff = b.center() - a.center();
			if (glm::dot(res.normal, center_diff) > 0)
			{
				res.mtv *= -1;
				res.normal *= -1;
			}
			return res;
		}
	}
	return {};
}

// returns edge that is most perpindicular to the normal
struct edge
{
	glm::vec2 max;
	glm::vec2 a, b;
	glm::vec2 normal;
};

edge find_best(const polygon_view &p, glm::vec2 normal)
{
	constexpr float epsilon = 1E-6f;

	float max = -std::numeric_limits<float>::infinity();
	std::size_t max_i;
	point max_pt;

	float s = 0;
	std::size_t prev = p.size() - 1;
	point prev_pt = p[prev];
	for (std::size_t i = 0; i < p.size(); prev = i++)
	{
		point cur_pt = p[i];
		auto proj = glm::dot(normal, cur_pt.pt);
		if (proj > max)
		{
			max_pt = cur_pt;
			max = proj;
			max_i = i;
		}

		s += (cur_pt.pt.x - prev_pt.pt.x) * (cur_pt.pt.y + prev_pt.pt.y);

		prev_pt = cur_pt;
	}

	std::size_t next, previous;

	if (max_i == p.size() - 1)
	{
		next = 0;
		previous = max_i - 1;
	}
	else if (max_i == 0)
	{
		next = 1;
		previous = p.size() - 1;
	}
	else
	{
		next = max_i + 1;
		previous = max_i - 1;
	}

	glm::vec2 next_pt = p.point(next);
	prev_pt = p[previous];

	float next_proj = glm::dot(max_pt.normal, normal);
	float prev_proj = glm::dot(prev_pt.normal, normal);

	if (std::abs(next_proj) > std::abs(prev_proj))
	{
		if (s >= 0)
			return {max_pt, max_pt.pt, next_pt, max_pt.normal};
		else
			return {max_pt, next_pt, max_pt.pt, max_pt.normal};
	}

	if (s >= 0)
		return {max_pt, prev_pt.pt, max_pt.pt, prev_pt.normal};
	else
		return {max_pt, max_pt.pt, prev_pt.pt, prev_pt.normal};
}

manifold clip(glm::vec2 a, glm::vec2 b, glm::vec2 edge, float projection)
{
	manifold res;
	float a_proj = glm::dot(a, edge);
	float b_proj = glm::dot(b, edge);

	unsigned int size = 0;
	if (a_proj >= projection)
		res.pts[size++] = a;
	if (b_proj >= projection)
		res.pts[size++] = b;
	
	if (size < 2)
	{
		glm::vec2 new_pt = b - a;
		new_pt *= (a_proj - projection) / (a_proj - b_proj);
		new_pt += a;

		res.pts[size] = new_pt;
	}

	return res;
}

#define DEBUG_MANIFOLD 0
#include <iomanip>

manifold contact_manifold(const polygon_view &a, const polygon_view &b, const collision &coll)
{
	#if DEBUG_MANIFOLD == 1
	std::cout << std::fixed << std::setprecision(4);
	std::cout << "A = " << a << '\n';
	std::cout << "B = " << b << '\n';
	#endif

	edge reference = find_best(b, coll.normal);
	edge incident = find_best(a, -coll.normal);

	#if DEBUG_MANIFOLD == 1
	std::cout << "\nRefMax = " << reference.max
			  << "\nRefA = " << reference.a
			  << "\nRefB = " << reference.b
			  << "\nIncMax = " << incident.max
			  << "\nIncA = " << incident.a
			  << "\nIncB = " << incident.b << std::endl;
	#endif

	auto edge = glm::normalize(reference.b - reference.a);
	auto clipped_pts = clip(incident.a, incident.b, edge, glm::dot(edge, reference.a));

	if (clipped_pts.size() < 2)
		return {};
	
	#if DEBUG_MANIFOLD == 1
	std::cout << "\nFirstClippedA = " << *clipped_pts.pts[0]
			  << "\nFirstClippedB = " << *clipped_pts.pts[1] << std::endl;
	#endif

	clipped_pts = clip(*clipped_pts.pts[0], *clipped_pts.pts[1], -edge, glm::dot(-edge, reference.b));

	if (clipped_pts.size() < 2)
		return {};

	#if DEBUG_MANIFOLD == 1
	std::cout << "\nSecondClippedA = " << *clipped_pts.pts[0]
			  << "\nSecondClippedB = " << *clipped_pts.pts[1] << std::endl;
	#endif

	glm::vec2 ref_norm{edge.y, -edge.x};

	float max_proj = glm::dot(ref_norm, reference.max);
	if (glm::dot(ref_norm, *clipped_pts.pts[0]) < max_proj)
	{
		if (glm::dot(ref_norm, *clipped_pts.pts[1]) < max_proj)
			clipped_pts.pts[0].reset();
		else
			*clipped_pts.pts[0] = *clipped_pts.pts[1];
		clipped_pts.pts[1].reset();
	}
	else if (glm::dot(ref_norm, *clipped_pts.pts[1]) < max_proj)
		clipped_pts.pts[1].reset();

	#if DEBUG_MANIFOLD == 1
	for (int i = 0; i < 2; ++i)
	{
		if (clipped_pts.pts[i])
			std::cout << "FinalClipped[" << i << "] = " << *clipped_pts.pts[i] << std::endl;
	}
	#endif
	
	return clipped_pts;
}

edge find_incident(const polygon_view &reference, const polygon_view &incident, glm::vec2 ref_normal)
{
	float min_dot = std::numeric_limits<float>::infinity();
	std::size_t min_i;
	for (std::size_t i = 0; i < incident.size(); ++i)
	{
		float cur_dot = glm::dot(ref_normal, incident.normal(i));
		if (cur_dot < min_dot)
		{
			min_dot = cur_dot;
			min_i = i;
		}
	}

	edge res;
	std::size_t next = min_i + 1 < incident.size() ? min_i + 1 : 0;
	res.a = incident.point(min_i);
	res.b = incident.point(next);

	return res;
}

float moment_of_inertia(const polygon_view &a)
{
	for (std::size_t i = 1; i < a.size(); ++i)
	{

	}

	return 0;
}