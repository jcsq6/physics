#ifndef BOUND_H
#define BOUND_H
#include <optional>
#include <algorithm>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

inline glm::mat3 rot2d(float rads)
{
	auto c = std::cos(rads);
	auto s = std::sin(rads);
	glm::mat3 mat({c, s, 0}, {-s, c, 0}, {0, 0, 1});
	return mat;
}

inline glm::vec2 rotate(glm::vec2 v, float angle)
{
	auto c = std::cos(angle);
	auto s = std::sin(angle);
	glm::vec2 res{
		c * v.x - s * v.y,
		s * v.x + c * v.y
	};

	return res;
}

struct bounding_box
{
	glm::vec2 min, max;
};

struct point
{
	glm::vec2 pt;
	glm::vec2 normal;

	operator glm::vec2() const { return pt; }
};

struct polygon_view;

struct collision
{
	glm::vec2 mtv;
	glm::vec2 normal;
	bool collides;

	inline collision() : mtv{}, normal{}, collides{} {}

	inline operator bool() const
	{
		return collides;
	}
};

class polygon
{
public:
	polygon() = default;

	template <typename T>
	inline polygon(std::initializer_list<glm::vec<2, T>> pts) : polygon()
	{
		assign(pts.begin(), pts.end());
	}

	template <typename InputIt>
	inline polygon(InputIt first, InputIt last) : polygon()
	{
		assign(first, last);
	}

	inline void reserve(std::size_t capacity)
	{
		m_pts.reserve(capacity);
	}

	inline void push_back(glm::vec2 pt)
	{
		m_pts.push_back({pt, {}});

		if (m_pts.size() == 1)
			return;

		// update the edge normals
		auto *last = &m_pts.back();
		set_normal(last - 1, last);
		set_normal(last, m_pts.data());

		calculate_center();
	}

	template <typename InputIt>
	inline void assign(InputIt first, InputIt last)
	{
		using vec_t = typename std::iterator_traits<InputIt>::value_type;
		static_assert(std::is_convertible_v<vec_t, glm::vec<2, float>>, "InputIt value_type must be convertible to a vec of size 2");

		m_pts.clear();

		if (first == last)
			return;
		
		m_pts.reserve(std::distance(first, last));

		m_pts.push_back({*(first++), {}});
		for (; first != last; ++first)
		{
			m_pts.push_back({*first, {}});

			// update the edge normals
			auto *last_pt = &m_pts.back();
			set_normal(last_pt - 1, last_pt);
		}

		// update the normal of the last point
		auto *last_pt = &m_pts.back();
		set_normal(last_pt, m_pts.data());

		calculate_center();
	}

	template <typename T>
	inline void assign(std::initializer_list<glm::vec<2, T>> pts)
	{
		assign(pts.begin(), pts.end());
	}

	inline void translate(glm::vec2 offset)
	{
		for (auto &pt : m_pts)
			pt.pt += offset;
		m_center += offset;
	}

	// move the first point to loc and all of the other ones follow suit
	inline void move_to(glm::vec2 loc)
	{
		move_to(loc, m_pts.front().pt);
	}

	// move in relation to origin
	inline void move_to(glm::vec2 loc, glm::vec2 origin)
	{
		glm::vec2 off = loc - origin;
		translate(off);
	}

	// rotate around origin
	inline void rotate(float rads, glm::vec2 origin)
	{
		auto mat = rot2d(rads);

		for (auto &pt : m_pts)
		{
			pt.pt -= origin;
			pt.pt = mat * glm::vec3(pt.pt, 1);
			pt.pt += origin;

			pt.normal = mat * glm::vec3(pt.normal, 1);
		}

		m_center -= origin;
		m_center = mat * glm::vec3(m_center, 1);
		m_center += origin;
	}

	// rotate about axis around origin
	inline void rotate(float rads, glm::vec3 axis, glm::vec2 origin)
	{
		auto mat = glm::rotate(glm::mat4(1.f), rads, axis);

		for (auto &pt : m_pts)
		{
			pt.pt -= origin;
			pt.pt = mat * glm::vec4(pt.pt, 0, 1);
			pt.pt += origin;

			pt.normal = mat * glm::vec4(pt.normal, 0, 1);
		}

		m_center -= origin;
		m_center = mat * glm::vec4(m_center, 0, 1);
		m_center += origin;
	}

	// scales points (does not translate)
	inline void scale(float scl)
	{
		for (auto &pt : m_pts)
			pt.pt *= scl;
	}

	glm::vec2 center() const
	{
		return m_center;
	}

	std::size_t size() const
	{
		return m_pts.size();
	}

	const point &operator[](std::size_t i) const
	{
		return m_pts[i];
	}

	auto pts_begin() const
	{
		return m_pts.begin();
	}

	auto pts_end() const
	{
		return m_pts.end();
	}

private:
	std::vector<point> m_pts;
	glm::vec2 m_center;

	void calculate_center()
	{
		glm::vec2 c{0, 0};
		for (const auto &pt : m_pts)
			c += pt.pt;
		c /= m_pts.size();

		m_center = c;
	}

	static inline void set_normal(point *first, point *second)
	{
		glm::vec2 perp{first->pt.y - second->pt.y, second->pt.x - first->pt.x};
		first->normal = glm::normalize(perp);
	}
};

struct polygon_view
{
	inline polygon_view(const polygon &_poly) : offset{}, poly{&_poly}, scale{1.f, 1.f}, angle{0.f} {}
	inline polygon_view(const polygon &_poly, glm::vec2 _offset, glm::vec2 _scale, float _angle) : offset{_offset}, poly{&_poly}, scale{_scale}, angle{_angle} {}
	glm::vec2 offset;
	glm::vec2 scale;
	const polygon *poly;
	float angle;

	std::size_t size() const
	{
		return poly->size();
	}

	glm::vec2 point(std::size_t i) const
	{
		return transform((*poly)[i].pt);
	}

	glm::vec2 normal(std::size_t i) const
	{
		return rotate((*poly)[i].normal, angle);
	}

	auto operator[](std::size_t i) const
	{
		auto p = (*poly)[i];
		p.pt = transform(p.pt);
		p.normal = rotate(p.normal, angle);
		return p;
	}

	glm::vec2 center() const
	{
		// center doesn't change with rotation
		return poly->center() * scale + offset;
	}

	glm::vec2 transform(glm::vec2 pt) const
	{
		// as if 
		// translate = {{1, 0, 0}, {0, 1, 0}, {offset.x, offset.y, 1}};
		// scale = {{scale.x, 0, 0}, {0, scale.y, 0}, {0, 0, 1}};
		// rotate = {{std::cos(angle), std::sin(angle), 0}, {-std::sin(angle), std::cos(angle), 0}, {0, 0, 1}};
		// transform = translate * rotate * scale_mat;
		// return transform * vec3((*poly)[i].pt, 1);
		auto c = std::cos(angle);
		auto s = std::sin(angle);
		glm::vec2 res{
			c * scale.x * pt.x - s * scale.y * pt.y + offset.x,
			s * scale.x * pt.x + c * scale.y * pt.y + offset.y
		};

		return res;
	}
};

#ifdef PHYSICS_DEBUG

#include <ostream>

template <glm::length_t size, typename T>
inline std::ostream &operator<<(std::ostream &stream, const glm::vec<size, T> &p)
{
	stream << '(';
	stream << p[0];
	for (int i = 1; i < size; ++i)
		stream << ", " << p[i];
	stream << ')';
	return stream;
}

inline std::ostream &operator<<(std::ostream &stream, const polygon_view &p)
{
	stream << "Polygon(";
	if (p.size())
	{
		stream << p.point(0);
		for (std::size_t i = 1; i < p.size(); ++i)
			stream << ", " << p.point(i);
	}
	stream << ')';
	return stream;
}

#endif

// returns collision with mtv to get a out of b or false if no collision
collision collides(const polygon_view &a, const polygon_view &b);

struct manifold
{
	std::optional<glm::vec2> pts[2];
	std::size_t size() const
	{
		return (!!pts[0] + !!pts[1]);
	}
};

manifold contact_manifold(const polygon_view &a, const polygon_view &b, const collision &coll);

float moment_of_inertia(const polygon_view &a);

template <typename Ait, typename Bit>
inline std::vector<glm::vec2> minkowski_subtract(Ait a_begin, Ait a_end, Bit b_begin, Bit b_end)
{
	auto a_size = std::distance(a_begin, a_end);
	auto b_size = std::distance(b_begin, b_end);

	std::vector<glm::vec2> res(a_size * b_size);
	
	std::size_t i = 0;
	for (auto ait = a_begin; ait != a_end; ++ait)
	{
		glm::vec2 a_pt = *ait;
		for (auto bit = b_begin; bit != b_end; ++bit, ++i)
			res[i] = a_pt - glm::vec2(*bit);
	}

	return res;
}

inline std::vector<glm::vec2> minkowski_subtract(const polygon_view &a, const polygon_view &b)
{
	std::vector<glm::vec2> res = minkowski_subtract(a.poly->pts_begin(), a.poly->pts_end(), b.poly->pts_begin(), b.poly->pts_end());
	return res;
}

// returns a regular polygon with n points and radius 1 around the origin (clockwise)
inline std::vector<glm::vec2> regular_polygon_pts(std::size_t n)
{
	if (!n)
		return {};
	
	float angle = 2 * glm::pi<float>() / n;
	glm::mat3 rot_mat = rot2d(angle);

	std::vector<glm::vec2> res(n);
	res[0] = {0, 1};
	
	// if even, rotate by half of the angle to make the bottom straight
	if (!(n & 1))
		res[0] = rot2d(angle / 2) * glm::vec3(res[0], 1);
	
	for (std::size_t i = 1; i < n; ++i)
		res[i] = rot_mat * glm::vec3(res[i - 1], 1);

	return res;
}

// returns a regular polygon with n points and radius 1 around the origin (clockwise)
inline polygon regular_polygon(std::size_t n)
{
	if (!n)
		return {};
	
	float angle = 2 * glm::pi<float>() / n;
	glm::mat3 rot_mat = rot2d(angle);

	polygon res;
	res.reserve(n);

	if (n & 1)
		res.push_back({0, 1});
	else // if even, rotate by half of the angle to make the bottom straight
		res.push_back(rot2d(angle / 2) * glm::vec3(0, 1, 1));
	
	for (std::size_t i = 1; i < n; ++i)
		res.push_back(rot_mat * glm::vec3(res[i - 1].pt, 1));

	return res;
}

#endif