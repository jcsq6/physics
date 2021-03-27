#pragma once
#include "vec2.h"
#include "vector_arithmetic.h"
#include <utility>

class poly;

struct collision {
	collision() : collides{false}{}
	collision(double overlap, const poly* q0, const poly* q1);
	collision(bool is_colliding) : collides{ is_colliding } {}

	operator bool() {
		return collides;
	}
	
	template<int which>
	vec2 min_translation_vec() {
		if (!which) return mtv0;
		return mtv1;
	}

private:
	bool collides;

	const poly* p0;
	const poly* p1;

	vec2 n0, n1, mtv0, mtv1;
};