#pragma once
#include "vec2.h"
#include "poly_flags.h"
#include <vector>
#include <utility>
#include <initializer_list>
#include <SDL2/SDL.h>

class poly {
public:
	poly();
	poly(const poly &p) noexcept;
	poly(poly &&p) noexcept;
	/// <summary>
	/// Constructor taking a std::initializer_list as points in a polygon
	/// </summary>
	/// <param name="ptl">-Points in polygon</param>
	poly(std::initializer_list<vec2<double>> ptl);

	/// <summary>
	/// Constructor taking a std::vector as points in a polygon, and flags for how the polygon should be handled
	/// </summary>
	/// <param name="ptl">-Points in polygon</param>
	/// <param name="checks_and_reqs">-Flags telling the class how to handle the points</param>
	poly(const std::vector<vec2<double>> &ptl, int checks_and_reqs);

	std::vector<vec2<double>>::const_iterator begin() const;
	std::vector<vec2<double>>::const_iterator end() const;

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
	bool update_pos(const vec2<double> &pos, const vec2<double> &ptInPoly);

	/// <summary>
	/// Rotates polygon around a point
	/// </summary>
	/// <param name="radians">-Angle in radians</param>
	/// <param name="center">-Center of rotation</param>
	void rotate(double radians, const vec2<double> &center);

	/// <summary>
	/// Sorts point in polygon. Only works if not done in constructor of polygon
	/// </summary>
	void sort_pts();

	/// <summary>
	/// Returns the center of the polygon. If there is no center, it will get the center
	/// </summary>
	vec2<double> center();

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

	/// <summary>
	/// Returns true if poly has a defined center
	/// GET_CENTER must be set
	/// </summary>
	bool has_center() const;
	/// <summary>
	/// Returns true if poly has checked and found the correct convex hull
	/// CHECK_HULL or IS_GUARANTEED must be set
	/// </summary>
	bool has_hull() const;
	/// <summary>
	/// Returns true if poly has been sorted
	/// SORT_PTS or IS_GUARANTEED must be set
	/// </summary>
	bool is_sorted() const;
	/// <summary>
	/// Returns true if poly has gone through checks and has not been overriden by the IS_GUARANTEED flag
	/// IS_GUARANTEED must not be set
	/// </summary>
	bool is_checked() const;

	poly operator+(const vec2<double> &disp) const;
	poly operator-(const vec2<double> &disp) const;
	poly operator+=(const vec2<double> &disp);
	poly operator-=(const vec2<double> &disp);
	bool operator==(const poly &q) const;
	bool operator!=(const poly &q) const;
	poly &operator=(const poly &p) noexcept;
	poly &operator=(poly &&p) noexcept;

	/// <summary>
	/// Creates a rectangle
	/// </summary>
	/// <param name="topLeft">-Top left point in rectangle</param>
	/// <param name="width">-Horizontal length of rectangle</param>
	/// <param name="height">-Vertical length of rectangle</param>
	/// <param name="flags">-Flags telling the class how to handle the points</param>
	static poly make_rect(const vec2<double> &topLeft, double width, double height, int flags);

	/// <summary>
	/// Creates a regular polygon with n number of sides
	/// </summary>
	/// <param name="center">-Center point of the polygon</param>
	/// <param name="radius">-Length from center to surrounding circle of the polygon</param>
	/// <param name="num_of_sides">-Number of sides the polygon has</param>
	/// <param name="checks_and_reqs">-Flags telling the class how to handle the points</param>
	static poly make_reg_poly(const vec2<double> &center, double radius, int num_of_sides, int checks_and_reqs);

	/// <summary>
	/// Takes two polygons and constructs a polygon path from the first polygon to the other
	/// Useful in collision detection 
	/// </summary>
	/// <param name="p1">-First polygon</param>
	/// <param name="p2">-Second polygon</param>
	/// <param name="flags">-Flags telling the class how to handle the points. NOTE: CHECK_HULL will always be set</param>
	/// <returns>Polygon representing the path from p1 to p2</returns>
	static poly get_poly_path(const poly &p1, const poly &p2, int flags);

	/// <summary>
	/// Collision detection between two polygons of any shape
	/// </summary>
	/// <returns>A std::pair where first is a bool telling if the polygons collided, and second is the collision normal (Will be undefined if polygons do not collide)</returns>
	static std::pair<bool, vec2<double>> is_colliding(const poly &q1, const poly &q2);

	static void draw_poly(SDL_Renderer *renderer, const poly &q);
private:
	vec2<double> c;
	std::vector<vec2<double>> pts;
	std::vector<vec2<double>> norms;
	size_t pt_count;
	int flags;

	void get_hull(const std::vector<vec2<double>> &set, const vec2<double> &b1, const vec2<double> &b2);
	void get_convex_hull(std::vector<vec2<double>> set);

	void get_center();
	void sort();
};


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