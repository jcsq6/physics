#include "poly.h"
#include <stdexcept>
#include <cmath>
#include <algorithm>

bool compare(const vec2 &p1, const vec2 &p2) {
	return p1.x < p2.x || (p1.x == p2.x && p1.y < p2.y);
}

inline int modulus(int a, int b) {
	int result = a % b;
	if (result >= 0) return result;
	else return result + b;
}

poly::poly() {
	c.x = 0;
	c.y = 0;
	flags = NONE;
}

poly::poly(const poly& p) noexcept : pts{ p.pts } {
	c = p.c;
	pt_count = p.pt_count;
	flags = p.flags;
}

poly::poly(poly&& p) noexcept : pts{ std::move(p.pts) } {
	c = p.c;
	pt_count = p.pt_count;
	flags = p.flags;
}

poly::poly(std::initializer_list<vec2> ptl) {
	get_convex_hull(ptl);
	pt_count = pts.size();

	sort();
	get_center();

	flags = FULL;
}

poly::poly(const a_vector<vec2>& ptl, int checks_and_reqs) {
	flags = checks_and_reqs;

	if (checks_and_reqs & CHECK_HULL) get_convex_hull(ptl);
	else pts = ptl;

	pt_count = pts.size();

	if (checks_and_reqs & SORT_PTS) sort();

	if (checks_and_reqs & GET_CENTER) get_center();

	if (checks_and_reqs & GET_AREA) get_area();
}

template<typename... T>
poly::poly(int checks_and_reqs, T... ptl){
	flags = checks_and_reqs;

	if (checks_and_reqs & CHECK_HULL) get_convex_hull({ ptl... });
	else pts = { ptl... };

	pt_count = pts.size();

	if (checks_and_reqs & SORT_PTS) sort();

	if (checks_and_reqs & GET_CENTER) get_center();

	if (checks_and_reqs & GET_AREA) get_area();
}

a_vector<vec2>::const_iterator poly::begin() const {
	return pts.begin();
}

a_vector<vec2>::const_iterator poly::end() const {
	return pts.end();
}

bool poly::update_pos(const vec2 &pos, const vec2 &ptInPoly) {
	//if ptInPoly is not a part of the poly, then it will skip and return false
	vec2 disp = pos - ptInPoly;

	int i = 0;
	for (; i < pt_count; i++) {
		if (ptInPoly == pts[i]) break;
	}
	if (i == pt_count && ptInPoly != c) return false;

	for (auto& pt : pts) {
		pt += disp;
	}

	c += disp;

	return true;
}

void poly::rotate(double radians, const vec2 &center) {
	for (auto& pt : pts) {
		pt.rotate(radians, center);
	}

	c.rotate(radians, center);
}

void poly::sort_pts(){
	sort();
}

vec2 poly::center() {
	if (!(flags & GET_CENTER)) {
		get_center();
		flags |= GET_CENTER;
	}
	return c;
}

vec2 poly::center() const {
	return c;
}

double poly::area() {
	if (!(flags & GET_AREA)) {
		get_area();
		flags |= GET_AREA;
	}
	return a;
}

double poly::area() const {
	return a;
}

size_t poly::size() const {
	return pt_count;
}

int poly::checks() const {
	return flags;
}

bool poly::has_collision_reqs() const {
	return (flags & COLLISION_REQS || flags & IS_GUARANTEED) && flags & GET_CENTER && !(flags & NO_COLLISION);
}

bool poly::has_drawing_reqs() const {
	return (flags & SORT_PTS || flags & IS_GUARANTEED) && !(flags & NO_DRAW);
}

bool poly::has_center() const {
	return flags & GET_CENTER;
}

bool poly::has_hull() const {
	return flags & CHECK_HULL;
}

bool poly::is_sorted() const {
	return flags & SORT_PTS;
}

bool poly::is_checked() const {
	return !(flags & IS_GUARANTEED);
}

poly poly::operator+(const vec2 &disp) const {
	return poly(*this) += disp;
}

poly poly::operator-(const vec2 &disp) const {
	return poly(*this) -= disp;
}

poly& poly::operator+=(const vec2 &disp) {
	pts += disp;
	c += disp;

	return *this;
}

poly& poly::operator-=(const vec2 &disp) {
	pts -= disp;
	c -= disp;

	return *this;
}

poly poly::operator*(const mat<double>& m) const {
	return poly(*this) *= m;
}

poly& poly::operator*=(const mat<double>& m) {
	pts *= m;
	c *= m;
	return *this;
}

bool poly::operator==(const poly &q) const {
	if (flags & SORT_PTS && q.flags & SORT_PTS) return q.pts == pts;
	throw std::logic_error{ "poly::operator==(const poly &q) ERROR: Can't compare unsorted points" };
}

bool poly::operator!=(const poly &q) const {
	if (flags & SORT_PTS && q.flags & SORT_PTS) return q.pts != pts;
	throw std::logic_error{ "poly::operator==(const poly &q) ERROR: Can't compare unsorted points" };
}

poly& poly::operator=(const poly& p) noexcept {
	c = p.c;
	pts = p.pts;
	pt_count = p.pt_count;
	flags = p.flags;
	return *this;
}

poly& poly::operator=(poly &&p) noexcept {
	c = p.c;
	pts = std::move(p.pts);
	pt_count = p.pt_count;
	flags = p.flags;
	return *this;
}

poly poly::make_rect(const vec2 &topLeft, double width, double height, int flags) {
	return poly({ topLeft, topLeft + vec2(width, 0), topLeft + vec2(0, height), topLeft + vec2(width, height) }, flags);
}

//no matter the flags, a GET_CENTER is always performed
poly poly::make_reg_poly(const vec2 &center, double radius, int num_of_sides, int checks_and_reqs) {
	a_vector<vec2> pts;
	pts.reserve(num_of_sides);
	for (int n = 1; n <= num_of_sides; n++) {
		pts.push_back({ radius * cos(n * 2 * M_PI / num_of_sides), radius * sin(n * 2 * M_PI / num_of_sides) });
	}
	poly p(pts, checks_and_reqs | GET_CENTER);
	p.update_pos(center, p.center());
	return p;
}

//no matter the flags, a CHECK_HULL is always performed
poly poly::get_poly_path(const poly &p1, const poly& p2, int flags) {
	a_vector<vec2> pts;
	pts.reserve(p1.pt_count + p2.pt_count);
	pts.insert(pts.begin(), p1.begin(), p1.end());
	pts.insert(pts.end(), p2.begin(), p2.end());
	flags |= CHECK_HULL;
	return poly(pts, flags);
}

struct edge_projection {
	double min, max;
	a_vector<vec2>::const_iterator min_pt, max_pt;
};

collision poly::is_colliding(const poly &q0, const poly &q1) {
	if (!q0.has_collision_reqs() || !q1.has_collision_reqs()) throw std::logic_error{ "Cannot guarentee collision success. Make sure to declare polys with POLY_FLAGS::COLLISION_REQS OR POLY_FLAGS::IS_GUARANTEED" };

	a_vector<vec2> norms;
	norms.reserve(q0.pt_count + q1.pt_count);

	vec2 edge;

	auto isSameDir = [&edge](const vec2& norm) { 
		return are_parallel(edge, norm);
	};

	for (int i = 0; i < q0.pt_count; i++) {
		edge = q0.pts[i] - q0.pts[(i + 1) % q0.pt_count];
		edge.perpindiculate();
		if (std::find_if(norms.begin(), norms.end(), isSameDir) == norms.end()) {
			//not neccesary for SAT, but neccessary for collision response
			edge.normalize();
			norms.push_back(edge);
		}
	}

	for (int i = 0; i < q1.pt_count; i++) {
		edge = q1.pts[i] - q1.pts[(i + 1) % q1.pt_count];
		edge.perpindiculate();
		if (std::find_if(norms.begin(), norms.end(), isSameDir) == norms.end()) {
			edge.normalize();
			norms.push_back(edge);
		}
	}

	vec2 center_average = (q0.center() + q1.center()) / 2;

	double d;
	double overlap = INFINITY;
	double o_t;
	edge_projection q0_proj, q1_proj;
	EDGE min_proj_q0, min_proj_q1;
	vec2* min_norm = nullptr;
	for (auto& norm : norms) {
		q0_proj.max = -INFINITY;
		q0_proj.min = INFINITY;
		q1_proj.max = -INFINITY;
		q1_proj.min = INFINITY;
		for (auto v = q0.begin(); v != q0.end(); v++) {
			d = dot(norm, *v);
			if (d < q0_proj.min) {
				q0_proj.min = d;
				q0_proj.min_pt = v;
			}
			if (d > q0_proj.max) {
				q0_proj.max = d;
				q0_proj.max_pt = v;
			}
		}
		for (auto v = q1.begin(); v != q1.end(); v++) {
			d = dot(norm, *v);
			if (d < q1_proj.min) {
				q1_proj.min = d;
				q1_proj.min_pt = v;
			}
			if (d > q1_proj.max) {
				q1_proj.max = d;
				q1_proj.max_pt = v;
			}
		}
		if ((q0_proj.min < q1_proj.max && q0_proj.min > q1_proj.min) || (q1_proj.min < q0_proj.max && q1_proj.min > q0_proj.min)) {
			o_t = std::min(q0_proj.max, q1_proj.max) - std::max(q0_proj.min, q1_proj.min);
			if (o_t < overlap) {
				overlap = o_t;

				min_proj_q0.first  = q0_proj.min_pt;
				min_proj_q0.second = q0_proj.max_pt;

				min_proj_q1.first  = q1_proj.min_pt;
				min_proj_q1.second = q1_proj.max_pt;
			}
			else if (o_t == overlap	&& 
				poly(GET_AREA, *min_proj_q0.first, *min_proj_q0.second, center_average).area() > poly(GET_AREA, *q0_proj.min_pt, *q0_proj.max_pt, center_average).area())
			{
				min_proj_q0.first = q0_proj.min_pt;
				min_proj_q0.second = q0_proj.max_pt;

				min_proj_q1.first = q1_proj.min_pt;
				min_proj_q1.second = q1_proj.max_pt;
			}
			continue;
		}
		else return collision(false);
	}
	return collision(&q0, &q1, overlap, min_proj_q0, min_proj_q1);
}

void poly::draw_poly(SDL_Renderer *renderer, const poly &q) {
	if (!q.has_drawing_reqs()) return;
	vec2 pt1;
	vec2 pt2;

	vec2 dir1;
	vec2 dir2;

	vec2 dest1;
	vec2 dest2;

	int p1i_init = 0;
	int p2i_init = 1;

	int d1i_init;
	int d2i_init;

	bool isInRad1{};
	bool isInRad2{};

	do {
		pt1 = q.pts[p1i_init];
		pt2 = q.pts[p2i_init];

		d1i_init = modulus(p1i_init - 1, (int)q.pt_count);
		d2i_init = modulus(p2i_init + 1, (int)q.pt_count);

		dest1 = q.pts[d1i_init];
		dest2 = q.pts[d2i_init];

		dir1 = dest1 - pt1;
		if (abs(dir1.x) > abs(dir1.y)) dir1 /= abs(dir1.x);
		else dir1 /= abs(dir1.y);

		dir2 = dest2 - pt2;
		if (abs(dir2.x) > abs(dir2.y)) dir2 /= abs(dir2.x);
		else dir2 /= abs(dir2.y);

		do {
			SDL_RenderDrawLineF(renderer, pt1.x, pt1.y, pt2.x, pt2.y);
			isInRad1 = isInRadius(pt1, dest1, 1);
			isInRad2 = isInRadius(pt2, dest2, 1);

			if (!isInRad1) pt1 += dir1;
			if (!isInRad2) pt2 += dir2;
		} while (!isInRad1 || !isInRad2);
		p1i_init = d1i_init;
		p2i_init = d2i_init;
	} while (abs(p1i_init - p2i_init) > 1);
}

void poly::get_hull(const std::vector<vec2> &set, const vec2 &b1, const vec2 &b2) {
	if (set.size() == 0 || set.size() == 1) return;

	std::vector<vec2> right;
	right.reserve(set.size());
	double max = -INFINITY;
	double area;
	vec2 maxpt;
	for (const auto& pt : set) {
		if (1 == sgn((pt.x - b1.x) * (b2.y - b1.y) - (pt.y - b1.y) * (b2.x - b1.x))) {
			right.push_back(pt);
			area = dot(b2 - b1, b2 - b1) * dot(pt - b1, pt - b1) - dot(b2 - b1, pt - b1) * dot(b2 - b1, pt - b1);
			if (area > max) {
				maxpt = pt;
				max = area;
			}
		}
	}
	if (!std::isinf(max)) pts.push_back(maxpt);

	get_hull(right, b1, maxpt);
	get_hull(right, maxpt, b2);
}

void poly::get_convex_hull(std::vector<vec2> set) {
	std::sort(set.begin(), set.end(), compare);

	pts.push_back(set.front());
	pts.push_back(set.back());

	get_hull(set, set.back(), set.front());
	get_hull(set, set.front(), set.back());
}

void poly::get_center() {
	double W = 0;
	double w = 0;
	for (int i = 0; i < pt_count; i++) {
		w = det(pts[i], pts[(i + 1) % pt_count]);
		W += w;
		c += (pts[i] + pts[(i + 1) % pt_count]) * w;
	}

	c /= 3 * W;
}

void poly::sort() {
	vec2 centroid(0, 0);
	for (const auto& pt : pts) {
		centroid += pt;
	}
	centroid /= (int)pt_count;
	std::sort(pts.begin(), pts.end(), [&centroid](vec2 &pt1, vec2 &pt2)
	{ return (pt1 - centroid).angle() < (pt2 - centroid).angle(); });
}

void poly::get_area() {
	a = 0;

	for (int i = 0; i < pt_count; i++) {
		a += (pts[modulus(i - 1, pt_count)].x + pts[i].x) * (pts[modulus(i - 1, pt_count)].y - pts[i].y);
	}

	a = abs(a / 2);
}