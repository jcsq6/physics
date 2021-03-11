#include "poly.h"
#include <stdexcept>
#include <SDL2/SDL.h>

bool compare(const vec2<double> &p1, const vec2<double> &p2) {
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

poly::poly(const poly &p) noexcept : pts{ p.pts }, norms{ p.norms } {
	c = p.c;
	pt_count = p.pt_count;
	flags = p.flags;
}

poly::poly(poly &&p) noexcept : pts{ std::move(p.pts) }, norms{ std::move(p.norms) }{
	c = p.c;
	pt_count = p.pt_count;
	flags = p.flags;
}

poly::poly(std::initializer_list<vec2<double>> ptl) {
	get_convex_hull(ptl);
	pt_count = pts.size();

	sort();
	get_center();

	flags = FULL;
}

poly::poly(const std::vector<vec2<double>> &ptl, int checks_and_reqs) {
	flags = checks_and_reqs;
	if (!(checks_and_reqs & IS_GUARANTEED)) {
		if (checks_and_reqs & CHECK_HULL) get_convex_hull(ptl);
		else pts = ptl;

		pt_count = pts.size();

		if (checks_and_reqs & SORT_PTS) sort();

		if (checks_and_reqs & GET_CENTER)	get_center();
	}
	else {
		pts = ptl;
		pt_count = pts.size();
	}
}

std::vector<vec2<double>>::const_iterator poly::begin() const {
	return pts.begin();
}

std::vector<vec2<double>>::const_iterator poly::end() const {
	return pts.end();
}

bool poly::update_pos(const vec2<double> &pos, const vec2<double> &ptInPoly) {
	//if ptInPoly is not a part of the poly, then it will skip and return false
	vec2<double> disp = pos - ptInPoly;

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

void poly::rotate(double radians, const vec2<double> &center) {
	for (auto& pt : pts) {
		pt.rotate(radians, center);
	}

	c.rotate(radians, center);
}

void poly::sort_pts(){
	sort();
}

vec2<double> poly::center() {
	if(flags & GET_CENTER) return c;
	else {
		get_center();
		return c;
	}
}

size_t poly::size() const {
	return pt_count;
}

int poly::checks() const {
	return flags;
}

bool poly::has_collision_reqs() const {
	return ((flags & SORT_PTS && flags & CHECK_HULL) || flags & IS_GUARANTEED) && !(flags & NO_COLLISION);
}

bool poly::has_drawing_reqs() const {
	return (flags & SORT_PTS || flags & IS_GUARANTEED) && !(flags & NO_DRAW);
}

bool poly::has_center() const {
	return flags & GET_CENTER;
}

bool poly::has_hull() const {
	return (flags & CHECK_HULL) || (flags & IS_GUARANTEED);
}

bool poly::is_sorted() const {
	return (flags & SORT_PTS) || (flags & IS_GUARANTEED);
}

bool poly::is_checked() const {
	return !(flags & IS_GUARANTEED);
}

poly poly::operator+(const vec2<double> &disp) const {
	poly temp = *this;
	for (auto& pt : temp.pts) {
		pt += disp;
	}
	temp.c += disp;

	return temp;
}

poly poly::operator-(const vec2<double> &disp) const {
	poly temp = *this;
	for (auto& pt : temp.pts) {
		pt -= disp;
	}
	temp.c -= disp;

	return temp;
}

poly poly::operator+=(const vec2<double> &disp) {
	for (auto& pt : pts) {
		pt += disp;
	}
	c += disp;

	return *this;
}

poly poly::operator-=(const vec2<double> &disp) {
	for (auto& pt : pts) {
		pt -= disp;
	}
	c -= disp;

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
	norms = p.norms;
	pt_count = p.pt_count;
	flags = p.flags;
	return *this;
}

poly& poly::operator=(poly &&p) noexcept {
	c = p.c;
	pts = std::move(p.pts);
	norms = std::move(p.norms);
	pt_count = p.pt_count;
	flags = p.flags;
	return *this;
}

poly poly::make_rect(const vec2<double> &topLeft, double width, double height, int flags) {
	return poly({ topLeft, topLeft + vec2<double>(width, 0), topLeft + vec2<double>(0, height), topLeft + vec2<double>(width, height) }, flags);
}

//no matter the flags, a GET_CENTER is always performed
poly poly::make_reg_poly(const vec2<double> &center, double radius, int num_of_sides, int checks_and_reqs) {
	std::vector<vec2<double>> pts;
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
	std::vector<vec2<double>> pts;
	pts.reserve(p1.pt_count + p2.pt_count);
	pts.insert(pts.begin(), p1.begin(), p1.end());
	pts.insert(pts.end(), p2.begin(), p2.end());
	flags |= CHECK_HULL;
	return poly(pts, flags);
}

std::pair<bool, vec2<double>> poly::is_colliding(const poly &q1, const poly &q2) {
	if (!q1.has_collision_reqs() || !q2.has_collision_reqs()) throw std::logic_error{ "Cannot guarentee collision success. Make sure to declare polys with POLY_FLAGS::COLLISION_REQS OR POLY_FLAGS::IS_GUARANTEED" };

	std::vector<vec2<double>> norms;
	norms.reserve(q1.pt_count + q2.pt_count);

	vec2<double> edge;

	auto isSameDir = [&edge](const vec2<double>& norm) { 
		return false;
	};

	for (int i = 0; i < q1.pt_count; i++) {
		edge = q1.pts[i] - q1.pts[(i + 1) % q1.pt_count];
		edge.perpindiculate();
		if (std::find_if(norms.begin(), norms.end(), isSameDir) == norms.end()) norms.push_back(edge);
	}

	for (int i = 0; i < q2.pt_count; i++) {
		edge = q2.pts[i] - q2.pts[(i + 1) % q2.pt_count];
		edge.perpindiculate();
		if (std::find_if(norms.begin(), norms.end(), isSameDir) == norms.end()) norms.push_back(edge);
	}

	double d = 0;
	double q1max, q1min, q2max, q2min;
	double maxIntersection = 0;
	vec2<double> collisionNormal;
	for (const auto& norm : norms) {
		q1max = -INFINITY;
		q1min = INFINITY;
		q2max = -INFINITY;
		q2min = INFINITY;
		for (const auto& vert : q1.pts) {
			d = dot(norm, vert);
			if (d > q1max) q1max = d;
			if (d < q1min) q1min = d;
		}
		for (const auto& vert : q2.pts) {
			d = dot(norm, vert);
			if (d > q2max) q2max = d;
			if (d < q2min) q2min = d;
		}
		if (q1min < q2max && q1min > q2min) {
			if (q2max - q1min > maxIntersection) {
				maxIntersection = q2max - q1min;
				collisionNormal = norm;
			}
			continue;
		}
		else if (q2min < q1max && q2min > q1min) {
			if (q1max - q2min > maxIntersection) {
				maxIntersection = q1max - q2min;
				collisionNormal = norm;
			}
			continue;
		}
		else return std::make_pair(false, vec2<double>());
	}
	return std::make_pair(true, collisionNormal);
}

void poly::draw_poly(SDL_Renderer *renderer, const poly &q) {
	if (!q.has_drawing_reqs()) return;
	vec2<double> pt1;
	vec2<double> pt2;

	vec2<double> dir1;
	vec2<double> dir2;

	vec2<double> dest1;
	vec2<double> dest2;

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

void poly::get_hull(const std::vector<vec2<double>> &set, const vec2<double> &b1, const vec2<double> &b2) {
	if (set.size() == 0 || set.size() == 1) return;

	std::vector<vec2<double>> right;
	right.reserve(set.size());
	double max = -INFINITY;
	double area;
	vec2<double> maxpt;
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

void poly::get_convex_hull(std::vector<vec2<double>> set) {
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
	vec2<double> centroid(0, 0);
	for (const auto& pt : pts) {
		centroid += pt;
	}
	centroid /= (int)pt_count;
	std::sort(pts.begin(), pts.end(), [&centroid](vec2<double> &pt1, vec2<double> &pt2)
	{ return (pt1 - centroid).angle() < (pt2 - centroid).angle(); });
}