#include "bound.h"

PHYSICS_BEG

inline bool contains_origin(std::vector<glm::vec2> &simplex, glm::vec2 &dir)
{
	switch (simplex.size())
	{
	case 1:
		return simplex[0] == glm::vec2{0, 0};
	case 2:
	{
		const glm::vec2 &a = simplex[0];
		const glm::vec2 &b = simplex[1];

		glm::vec2 ab = b - a;
		glm::vec2 norm = { ab.y, -ab.x };
		if (glm::dot(ab, norm) < 0)
			norm *= -1;
		
		if (glm::dot(norm, -a) > 0)
			dir = norm;
		else
			dir = -norm;

		return false;
	}
	case 3:
	{
		const glm::vec2 &a = simplex[2];
		const glm::vec2 &b = simplex[1];
		const glm::vec2 &c = simplex[0];

		glm::vec2 ab = b - a;
		glm::vec2 ac = c - a;
		
		glm::vec2 ab_norm = { ab.y, -ab.x};
		if (glm::dot(ac, ab_norm) > 0)
			ab_norm *= -1;
		
		glm::vec2 ac_norm = { ac.y, -ac.x };
		if (glm::dot(ab, ac_norm) > 0)
			ac_norm *= -1;
		
		if (glm::dot(ab_norm, -a) > 0)
		{
			simplex.erase(simplex.begin());
			dir = ab_norm;
		}
		else if (glm::dot(ac_norm, -a) > 0)
		{
			simplex.erase(simplex.begin() + 1);
			dir = ac_norm;
		}
		else
			return true;
		
		return false;
	}
	default:
		return false;
	}
}

struct edge
{
	glm::vec2 norm;
	float dist;
	unsigned int i;
};

edge closest_edge(std::vector<glm::vec2> &simplex)
{
	edge closest;
	closest.dist = std::numeric_limits<float>::infinity();

	for (unsigned int i = 0; i < simplex.size(); ++i)
	{
		const glm::vec2 &a = simplex[i];
		const glm::vec2 &b = simplex[(i + 1) % simplex.size()];
		glm::vec2 ab = b - a;
		glm::vec2 norm = { ab.y, -ab.x };
		if (glm::dot(a, norm) < 0)
			norm *= -1;
		norm = glm::normalize(norm);

		if (float dist = glm::dot(a, norm); dist < closest.dist)
		{
			closest.norm = norm;
			closest.dist = dist;
			closest.i = i;
		}
	}

	return closest;
}

// expanding polytope algorithm
collision epa(std::vector<glm::vec2> &simplex, const shape_view &a, const shape_view &b)
{
	while (true)
	{
		edge edge = closest_edge(simplex);
		auto a_support = a.support(edge.norm);
		auto b_support = b.support(-edge.norm);
		glm::vec2 new_pt = a_support - b_support;
		float dist = glm::dot(new_pt, edge.norm);

		if (dist - edge.dist < 1e-6f)
			return { edge.norm, dist, a_support, b_support };
		else
			simplex.insert(simplex.begin() + edge.i + 1, new_pt);
	}
}

// returns collision with mtv to get a out of b or false if no collision
collision collides(const shape_view &a, const shape_view &b)
{
	glm::vec2 dir{1, 0};
	thread_local std::vector<glm::vec2> simplex(3);
	simplex.clear();

	simplex.push_back(a.support(dir) - b.support(-dir));
	dir = -simplex[0];

	while (true)
	{
		glm::vec2 new_pt = a.support(dir) - b.support(-dir);
		if (glm::dot(new_pt, dir) <= 0)
			return {}; // doesn't collide

		simplex.push_back(new_pt);
		if (contains_origin(simplex, dir))
			return epa(simplex, a, b); // collides
	}
}

// TODO
float moment_of_inertia(const shape_view &a)
{
	for (std::size_t i = 1; i < a.size(); ++i)
	{

	}

	return 0;
}

PHYSICS_END