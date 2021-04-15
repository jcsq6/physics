#pragma once
#include "mat.h"
#include "collision.h"
#include "poly_flags.h"
#include <initializer_list>
#include <algorithm>
#include <SDL2/SDL.h>

class poly {
public:
	poly();
	poly(const poly &p) noexcept;
	poly(poly&& p) noexcept;
	poly(std::initializer_list<vec2> ptl);

	template<typename T>
	explicit poly(T&& ptl, int checks_and_reqs) {
		flags = checks_and_reqs;

		if (flags & CHECK_HULL) get_convex_hull(std::forward<T>(ptl));
		else pts = std::forward<T>(ptl);

		pt_count = pts.size();

		run_checks_get_reqs(flags);
	}

	template<typename... T>
	explicit poly(int checks_and_reqs, T... ptl) {
		flags = checks_and_reqs;

		if (flags & CHECK_HULL) get_convex_hull(a_vector<vec2>{ ptl... });
		else pts = { ptl... };

		pt_count = pts.size();

		run_checks_get_reqs(flags);
	}

	const a_vector<vec2>& points() const;
	const a_vector<edge>& normals() const;
	const a_vector<edge>& normals();

	const vec2& support(const vec2& axis) const;

	a_vector<vec2>::const_iterator begin() const;
	a_vector<vec2>::const_iterator end() const;

	/// <summary>
	/// Makes a polygon that has been handled in some way by the predicate
	/// </summary>
	/// <param name="predicate">-A function, functor, or lamda with ONE vec2 argument that returns a vec2</param>
	/// <param name="succeding_checks">-Flags telling the class how to handle the points in the new polygon. FLAGS ARE NOT INHERITED</param>
	/// <returns>Poly</returns>
	template<class O> 
	poly transform(O predicate, int succeding_checks) const {
		poly p = *this;

		for (auto& pt : p.pts) {
			pt = predicate(pt);
		}

		p.flags = succeding_checks;

		if (!(p.flags & IS_GUARANTEED)) {
			if (p.flags & CHECK_HULL) p.get_convex_hull(p.pts);

			p.run_checks_get_reqs(p.flags);
		}
		else p.flags = flags;
		return p;
	}

	/// <summary>
	/// Moves the polygon to vec2 pos
	/// </summary>
	/// <param name="pos">-Pos that the polygon will move to</param>
	/// <param name="ptInPoly">-Point in polygon that will be moved to pos. All other point will move in reference</param>
	void update_pos(const vec2& pos, const vec2& pt_in_poly);

	void rotate(double radians, const vec2 &center);

	void set_local_rotation(double radians, const vec2& pt_in_poly_0, const vec2& pt_in_poly_1);
	double local_rotation(const vec2& pt_in_poly_0, const vec2& pt_in_poly_1) const;

	void add_flags(int checks_and_reqs);

	const vec2& center();

	const vec2& center() const;

	double area();

	double area() const;

	const vec2& support_pt(const vec2& axis);

	/// <summary>
	/// Gets number of points in polygon
	/// </summary>
	size_t size() const;

	/// <summary>
	/// Get internal polygon check flags
	/// </summary>
	int checks() const;

	/// <summary>
	/// Returns true if poly meets collision requirements. 
	/// One of the following flags must be set:
	/// COLLISION_REQS, IS_GUARANTEED
	/// The following flags must not be set:
	/// NO_COLLISION
	/// </summary>
	bool has_collision_reqs() const;

	/// <summary>
	/// Returns true if poly meets drawing requirements. 
	/// One of the following flags must be set:
	/// DRAW_REQS, SORT_PTS, IS_GUARANTEED
	/// The following flags must not be set:
	/// NO_DRAW
	/// </summary>
	bool has_drawing_reqs() const;

	bool has_center() const;

	bool has_hull() const;

	bool is_sorted() const;
	
	bool is_checked() const;

	const vec2& operator[](int i) const;

	poly operator+(const vec2 &disp) const;
	poly operator-(const vec2 &disp) const;
	poly& operator+=(const vec2 &disp);
	poly& operator-=(const vec2 &disp);
	poly operator*(const mat<double>& m) const;
	poly& operator*=(const mat<double>& m);
	bool operator==(const poly &q) const;
	bool operator!=(const poly &q) const;
	poly& operator=(const poly &p) noexcept;
	poly& operator=(poly &&p) noexcept;

	static poly make_rect(const vec2& topLeft, double width, double height, int checks_and_reqs = default_flags);
	static poly make_reg_poly(const vec2& center, double radius, int num_of_sides, double radians = 0, int checks_and_reqs = default_flags);
	static poly make_line(const vec2& strt, const vec2& end, double thickness, int checks_and_reqs = default_flags);
	static poly make_pt(const vec2& pt, double size, int checks_and_reqs = default_flags);

	/// <summary>
	/// Takes two polygons and constructs a polygon path from the first polygon to the other
	/// Useful in collision detection 
	/// </summary>
	/// <param name="p1">-First polygon</param>
	/// <param name="p2">-Second polygon</param>
	/// <param name="checks_and_reqs">-Flags telling the class how to handle the points. NOTE: CHECK_HULL will always be set</param>
	/// <returns>Polygon representing the path from p1 to p2</returns>
	static poly get_poly_path(const poly &p1, const poly &p2, int checks_and_reqs);

	static collision is_colliding(const poly &q0, const poly &q1);

	static void draw_poly(SDL_Renderer* renderer, const poly &q);

	template<typename... Ps>
	static void draw_polys(SDL_Renderer* renderer, Ps&&... polys) {
		(draw_poly(renderer, std::forward<Ps>(polys)), ...);
	}

	static void set_default_flags(int flags);
private:
	vec2 c;
	a_vector<vec2> pts;
	a_vector<edge> norms;
	size_t pt_count;
	double a;
	int flags;

	inline static int default_flags = FULL;

	template<typename T>
	void get_hull(const T& set, const vec2 &b1, const vec2 &b2) {
		if (set.size() == 0 || set.size() == 1) return;

		a_vector<vec2> right;
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

	template<typename T>
	void get_convex_hull(T&& set) {
		auto compare = [](const vec2& p1, const vec2& p2) {
			return p1.x < p2.x || (p1.x == p2.x && p1.y < p2.y);
		};

		T pt_set = std::forward<T>(set);

		pts.clear();

		std::sort(std::begin(pt_set), std::end(pt_set), compare);

		pts.push_back(*std::begin(pt_set));
		pts.push_back(*std::prev(std::end(pt_set)));

		get_hull(pt_set, *std::prev(std::end(pt_set)), *std::begin(pt_set));
		get_hull(pt_set, *std::begin(pt_set), *std::prev(std::end(pt_set)));
	}

	void sort();

	void get_center();

	void get_area();

	void get_normals();

	void run_checks_get_reqs(int checks_and_reqs);
};

inline vec2 collision::min_translation_vec(int which) const {
	if (!which) return normalize(p0->center() - p1->center()) * depth;
	return normalize(p1->center() - p0->center()) * depth;
}

inline vec2 collision::min_translation_vec(const poly* which) const {
	if (which == p0) return normalize(p0->center() - p1->center()) * depth;
	return normalize(p1->center() - p0->center()) * depth;
}