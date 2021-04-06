#pragma once
#include "mat.h"
#include "collision.h"
#include "poly_flags.h"
#include <initializer_list>
#include <SDL2/SDL.h>

class poly {
public:
	poly();
	poly(const poly &p) noexcept;
	poly(poly &&p) noexcept;
	poly(std::initializer_list<vec2> ptl);

	poly(const a_vector<vec2> &ptl, int checks_and_reqs);

	template<typename... T>
	poly(int checks_and_reqs, T... pts);

	a_vector<vec2>::const_iterator begin() const;
	a_vector<vec2>::const_iterator end() const;

	/// <summary>
	/// Makes a polygon that has been handled in some way by the predicate
	/// </summary>
	/// <param name="predicate">-A function, functor, or lamda with ONE vec2 argument that returns a vec2</param>
	/// <param name="succeding_checks">-Flags telling the class how to handle the points in the new polygon. FLAGS ARE NOT INHERITED</param>
	/// <returns>new Poly</returns>
	template<class O> poly convert(O predicate, int succeding_checks) const;

	/// <summary>
	/// Moves the polygon to vec2 pos
	/// </summary>
	/// <param name="pos">-Pos that the polygon will move to</param>
	/// <param name="ptInPoly">-Point in polygon that will be moved to pos. All other point will move in reference</param>
	/// <returns>Bool indicating success or failure</returns>
	bool update_pos(const vec2 &pos, const vec2 &ptInPoly);

	void rotate(double radians, const vec2 &center);

	void sort_pts();

	vec2 center();

	vec2 center() const;

	double area();

	double area() const;

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

	static poly make_rect(const vec2 &topLeft, double width, double height, int flags);

	static poly make_reg_poly(const vec2 &center, double radius, int num_of_sides, int checks_and_reqs);

	/// <summary>
	/// Takes two polygons and constructs a polygon path from the first polygon to the other
	/// Useful in collision detection 
	/// </summary>
	/// <param name="p1">-First polygon</param>
	/// <param name="p2">-Second polygon</param>
	/// <param name="flags">-Flags telling the class how to handle the points. NOTE: CHECK_HULL will always be set</param>
	/// <returns>Polygon representing the path from p1 to p2</returns>
	static poly get_poly_path(const poly &p1, const poly &p2, int flags);

	static collision is_colliding(const poly &q0, const poly &q1);

	static void draw_poly(SDL_Renderer *renderer, const poly &q);
private:
	vec2 c;
	a_vector<vec2> pts;
	size_t pt_count;
	double a;
	int flags;

	void get_hull(const std::vector<vec2> &set, const vec2 &b1, const vec2 &b2);
	void get_convex_hull(std::vector<vec2> set);

	void get_center();
	void sort();

	void get_area();
};

inline void draw_pt(SDL_Renderer* renderer, const vec2& pt, int scale = 1) {
	for (int x = (int)pt.x; x < pt.x + scale; x++) {
		for (int y = (int)pt.y; y < pt.y + scale; y++) {
			SDL_RenderDrawPoint(renderer, x, y);
		}
	}
}

template<class O> poly poly::convert(O predicate, int succeding_checks) const {
	poly p = *this;

	for (auto& pt : p.pts) {
		pt = predicate(pt);
	}

	p.flags = succeding_checks;

	if (!(succeding_checks & IS_GUARANTEED)) {
		if (succeding_checks & CHECK_HULL) p.get_convex_hull(p.pts);

		if (succeding_checks & SORT_PTS) p.sort();

		if (succeding_checks & GET_CENTER) p.get_center();
	}
	else p.flags = flags;
	return p;
}

inline collision::collision(const poly* q0, const poly* q1, double overlap, EDGE col_edge_0, EDGE col_edge_1) : 
	collides{ true }, p0{ q0 }, p1{ q1 }, e0{ col_edge_0 }, e1{ col_edge_1 }
{
	n0 = p1->center() - p0->center();
	n1 = p0->center() - p1->center();

	//it's not backwards, this is neccesary
	mtv0 = normalize(n1) * overlap;
	mtv1 = normalize(n0) * overlap;
}