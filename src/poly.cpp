#include "poly.h"
#include <stdexcept>
#include <cmath>

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

poly::poly(const poly& p) noexcept : pts{ p.pts }, norms{ p.norms } {
	c = p.c;
	pt_count = p.pt_count;
	flags = p.flags;
}

poly::poly(poly&& p) noexcept : pts{ std::move(p.pts) }, norms{ std::move(p.norms) } {
	c = p.c;
	pt_count = p.pt_count;
	flags = p.flags;
}

poly::poly(std::initializer_list<vec2> ptl) {
	flags = default_flags;

	if (flags & CHECK_HULL) get_convex_hull(a_vector<vec2>(ptl));
	else pts = ptl;

	pt_count = pts.size();

	run_checks_get_reqs(flags);
}

const a_vector<vec2>& poly::points() const {
	return pts;
}

const a_vector<edge>& poly::normals() const {
	return norms;
}

const a_vector<edge>& poly::normals() {
	if (!(flags & GET_NORMALS)) get_normals();
	return norms;
}

const vec2& poly::support(const vec2& axis) const {
	double max = -INFINITY;
	double d;
	const vec2* max_pt = nullptr;

	for (const auto& pt : pts) {
		d = dot(pt, axis);

		if (d > max) {
			max = d;
			max_pt = &pt;
		}
	}

	return *max_pt;
}

a_vector<vec2>::const_iterator poly::begin() const {
	return pts.begin();
}

a_vector<vec2>::const_iterator poly::end() const {
	return pts.end();
}

void poly::update_pos(const vec2 &pos, const vec2& pt_in_poly) {
	//if ptInPoly is not a part of the poly, then it throw an exception
	if (std::find(pts.begin(), pts.end(), pt_in_poly) == pts.end() && pt_in_poly != c) throw std::out_of_range("pt not in poly");

	vec2 disp = pos - pt_in_poly;

	for (auto& pt : pts) {
		pt += disp;
	}

	c += disp;
}

void poly::rotate(double radians, const vec2& center) {
	for (auto& pt : pts) {
		pt.rotate(radians, center);
	}

	if (flags & GET_NORMALS) {
		for (auto& edge : norms) {
			edge.norm.rotate(radians, center);
		}
	}

	c.rotate(radians, center);
}

void poly::set_local_rotation(double radians, const vec2& pt_in_poly_0, const vec2& pt_in_poly_1){
	auto it_0 = std::find(pts.begin(), pts.end(), pt_in_poly_0);
	auto it_1 = std::find(pts.begin(), pts.end(), pt_in_poly_1);
	if ((it_0 != pts.end() || pt_in_poly_0 == center()) && (it_1 != pts.end() || pt_in_poly_1 == center())) {
		rotate(radians - (pt_in_poly_1 - pt_in_poly_0).angle(), pt_in_poly_0);
	}
	else {
		throw std::out_of_range("pt not in poly");
	}
}

double poly::local_rotation(const vec2& pt_in_poly_0, const vec2& pt_in_poly_1) const {
	auto it_0 = std::find(pts.begin(), pts.end(), pt_in_poly_0);
	auto it_1 = std::find(pts.begin(), pts.end(), pt_in_poly_1);
	if ((it_0 != pts.end() || pt_in_poly_0 == center()) && (it_1 != pts.end() || pt_in_poly_1 == center())) {
		return (pt_in_poly_1 - pt_in_poly_0).angle();
	}
	throw std::out_of_range("pt not in poly");
}

void poly::add_flags(int checks_and_reqs){
	checks_and_reqs &= ~flags;

	if (checks_and_reqs & CHECK_HULL) {
		get_convex_hull(pts);
		pt_count = pts.size();
	}

	run_checks_get_reqs(checks_and_reqs);

	flags |= checks_and_reqs;
}

const vec2& poly::center() {
	if (!(flags & GET_CENTER)) {
		get_center();
		flags |= GET_CENTER;
	}
	return c;
}

const vec2& poly::center() const {
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
	return (flags & COLLISION_REQS || flags & IS_GUARANTEED) && flags & GET_CENTER && flags & GET_NORMALS && !(flags & NO_COLLISION);
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

const vec2& poly::operator[](int i) const {
	return pts[i];
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
	if(flags & GET_NORMALS)	get_normals();
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

poly poly::make_rect(const vec2 &topLeft, double width, double height, int checks_and_reqs) {
	return poly(checks_and_reqs, topLeft, topLeft + vec2(width, 0), topLeft + vec2(0, height), topLeft + vec2(width, height));
}

//no matter the flags, a GET_CENTER is always performed
poly poly::make_reg_poly(const vec2 &center, double radius, int num_of_sides, double radians, int checks_and_reqs) {
	a_vector<vec2> pts;
	pts.reserve(num_of_sides * 2);
	//start at n = 2 and go 1 past the num of sides to avoid unnecesary sorting
	for (int n = 1; n <= num_of_sides; n++) {
		pts.push_back({ radius * cos(n * 2 * M_PI / num_of_sides), radius * sin(n * 2 * M_PI / num_of_sides) });
	}
	poly p(std::move(pts), (checks_and_reqs | GET_CENTER) & ~GET_NORMALS & ~CHECK_HULL);
	p.set_local_rotation(radians, p[0], p[1]);
	if (checks_and_reqs & GET_NORMALS) p.add_flags(GET_NORMALS);
	p.update_pos(center, p.center());
	p.flags |= CHECK_HULL;
	return p;
}

poly poly::make_line(const vec2& strt, const vec2& end, double thickness, int checks_and_reqs){
	return poly(checks_and_reqs, strt - vec2(thickness / 2, thickness / 2), strt + vec2(thickness / 2, thickness / 2), end - vec2(thickness / 2, thickness / 2), end + vec2(thickness / 2, thickness / 2));
}

poly poly::make_pt(const vec2& pt, double size, int checks_and_reqs){
	return poly(checks_and_reqs, pt - vec2(size / 2, size / 2), pt + vec2(size / 2, -size / 2), pt - vec2(size / 2, -size / 2), pt + vec2(size / 2, size / 2));
}

//no matter the flags, a CHECK_HULL is always performed
poly poly::get_poly_path(const poly &p1, const poly& p2, int checks_and_reqs) {
	a_vector<vec2> pts;
	pts.reserve(p1.pt_count + p2.pt_count);
	pts.insert(pts.begin(), p1.begin(), p1.end());
	pts.insert(pts.end(), p2.begin(), p2.end());
	checks_and_reqs |= CHECK_HULL;
	return poly(pts, checks_and_reqs);
}
inline const edge* find_incident(const poly* incident_owner, const edge& reference_edge) {
	double min_dot = INFINITY;
	double d;
	const edge* incident_edge = nullptr;
	for (const auto& e : incident_owner->normals()) {
		d = dot(reference_edge.norm, e.norm);
		if (d < min_dot) {
			min_dot = d;
			incident_edge = &e;
		}
	}
	return incident_edge;
}

inline std::pair<const edge*, double> find_min(const poly* q0, const poly* q1) {
	double d;
	auto min_edge = std::make_pair<const edge*, double>(nullptr, -INFINITY);
	for (const auto& e : q0->normals()) {
		d = dot(q1->support(-e.norm) - *e.p1, e.norm);
		if (d > 0) return { nullptr, 0 };
		else if (d > min_edge.second) {
			min_edge.first = &e;
			min_edge.second = d;
		}
	}
	return min_edge;
}


collision poly::is_colliding(const poly &q0, const poly &q1) {
	if (!q0.has_collision_reqs() || !q1.has_collision_reqs()) throw std::logic_error{ "Cannot guarentee collision success. Make sure to declare polys with POLY_FLAGS::COLLISION_REQS OR POLY_FLAGS::IS_GUARANTEED" };

	if (auto q0_proj = find_min(&q0, &q1); q0_proj.second <= 0 && q0_proj.first) {
		if (auto q1_proj = find_min(&q1, &q0); q1_proj.second <= 0 && q1_proj.first) {
			if (q0_proj.second > q1_proj.second)
				return collision(&q0, &q1, q0_proj.second, *q0_proj.first, *find_incident(&q1, *q0_proj.first));
			else return collision(&q1, &q0, q1_proj.second, *q1_proj.first, *find_incident(&q0, *q1_proj.first));
		}
	}
	return collision(false);
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

void poly::set_default_flags(int flags){
	default_flags = flags;
}

void poly::sort() {
	vec2 centroid(0, 0);
	for (const auto& pt : pts) {
		centroid += pt;
	}
	centroid /= (int)pt_count;
	std::sort(pts.begin(), pts.end(), [&centroid](vec2& pt1, vec2& pt2)
	{ return (pt1 - centroid).angle() < (pt2 - centroid).angle(); });
}

void poly::get_center() {
	double W = 0;
	double w = 0;
	c.x = c.y = 0;
	if (pt_count > 2) {
		for (int i = 0; i < pt_count; i++) {
			w = det(pts[i], pts[(i + 1) % pt_count]);
			W += w;
			c += (pts[i] + pts[(i + 1) % pt_count]) * w;
		}
		c /= 3 * W;
	}
	else {
		for (int i = 0; i < pt_count; i++) {
			c += pts[i];
		}
		c /= pt_count;
	}
}

void poly::get_area() {
	if (!(flags & SORT_PTS)) sort();

	a = 0;

	for (int i = 0; i < pt_count; i++) {
		a += (pts[modulus(i - 1, pt_count)].x + pts[i].x) * (pts[modulus(i - 1, pt_count)].y - pts[i].y);
	}

	a = abs(a / 2);
}

void poly::get_normals() {
	if (!(flags & SORT_PTS)) sort();

	edge e;

	for (int i = 0; i < pt_count; i++) {
		e.p1 = pts.begin() + i;
		e.p2 = pts.begin() + ((i + 1) % pt_count);
		e.norm = perpindiculate(normalize(*e.p1 - *e.p2));
		norms.push_back(e);
	}
}

void poly::run_checks_get_reqs(int checks_and_reqs){
	if (checks_and_reqs & SORT_PTS) sort();

	if (checks_and_reqs & GET_CENTER) get_center();

	if (checks_and_reqs & GET_AREA) get_area();

	if (checks_and_reqs & GET_NORMALS) get_normals();
}