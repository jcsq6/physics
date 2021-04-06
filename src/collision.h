#pragma once
#include "vec2.h"
#include "vector_arithmetic.h"
#include <utility>

#define EDGE std::pair<a_vector<vec2>::const_iterator, a_vector<vec2>::const_iterator>

class poly;

struct collision {
	collision(const poly* q0, const poly* q1, double overlap, EDGE col_edge_0, EDGE col_edge_1);
	collision(bool is_colliding = false) : collides{ is_colliding } {}

	operator bool() {
		return collides;
	}
	
	vec2& min_translation_vec(int which) {
		if (!which) return mtv0;
		return mtv1;
	}

	vec2& min_translation_vec(const poly* which) {
		if (which == p0) return mtv0;
		return mtv1;
	}

	vec2& normal(int which) {
		if (!which) return n0;
		return n1;
	}

	vec2& normal(const poly* which) {
		if (which == p0) return n0;
		return n1;
	}

	EDGE get_collision_edge(int which) {
		if (!which) return e0;
		return e1;
	}

	EDGE get_collision_edge(const poly* which) {
		if (which == p0) return e0;
		return e1;
	}

private:
	bool collides;

	const poly* p0;
	const poly* p1;

	EDGE e0, e1;

	vec2 n0, n1, mtv0, mtv1;
};