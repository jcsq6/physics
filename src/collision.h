#pragma once
#include "vec2.h"
#include "vector_arithmetic.h"
#include <utility>

class poly;

struct edge {
	a_vector<vec2>::const_iterator p1;
	a_vector<vec2>::const_iterator p2;

	vec2 norm;
};

struct collision {
	collision(const poly* q0, const poly* q1, double overlap, const edge& reference, const edge& incident) :
		collides{ true }, depth{ overlap }, p0{ q0 }, p1{ q1 }, r{ reference }, i{ incident }
	{

	}
	collision(bool is_colliding = false) : collides{ is_colliding } {}

	operator bool() {
		return collides;
	}
	
	vec2 min_translation_vec(int which) const;
	vec2 min_translation_vec(const poly* which) const;

	const vec2& normal() const {
		return r.norm;
	}

	const edge& reference() const {
		return r;
	}

	const edge& incident() const {
		return i;
	}

	double overlap() {
		return depth;
	}

	const vec2& max_collision_pt(int which) const {
		if (!which) return q0_max_col;
		return q1_max_col;
	}
	const vec2& max_collision_pt(const poly* which) const {
		if (which == p0) return q0_max_col;
		return q1_max_col;
	}

	const poly* q0() const {
		return p0;
	}

	const poly* q1() const {
		return p1;
	}

private:
	bool collides;

	double depth;

	const poly* p0;
	const poly* p1;

	vec2 q0_max_col;
	vec2 q1_max_col;

	edge r;
	edge i;
};