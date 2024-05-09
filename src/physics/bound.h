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

class polygon_view;

struct collision
{
	glm::vec2 mtv;
	glm::vec2 normal;
	bool collides;

	inline collision() : mtv{}, normal{}, collides{} {}

	inline operator bool() const { return collides;	}
};

class polygon
{
public:
	polygon() = default;

	template <typename T>
	polygon(std::initializer_list<glm::vec<2, T>> pts) : polygon() { assign(pts.begin(), pts.end()); }

	template <typename InputIt>
	polygon(InputIt first, InputIt last) : polygon() { assign(first, last); }

	void reserve(std::size_t capacity)
	{
		m_pts.reserve(capacity);
	}

	void push_back(glm::vec2 pt)
	{
		m_pts.push_back(pt);
		m_center = (m_center * float(m_pts.size() - 1) + pt) / (float)m_pts.size();
	}

	template <typename InputIt>
	void assign(InputIt first, InputIt last)
	{
		using vec_t = typename std::iterator_traits<InputIt>::value_type;
		static_assert(std::is_convertible_v<vec_t, glm::vec<2, float>>, "InputIt value_type must be convertible to a vec of size 2");

		m_pts.clear();
		m_pts.reserve(std::distance(first, last));
		
		glm::vec2 c{0, 0};
		for (; first != last; ++first)
		{
			m_pts.push_back(*first);
			c += *first;
		}

		if (m_pts.size())
			c /= m_pts.size();
		
		m_center = c;
	}

	template <typename T>
	void assign(std::initializer_list<glm::vec<2, T>> pts) { assign(pts.begin(), pts.end()); }

	std::size_t size() const { return m_pts.size(); }

	glm::vec2 center() const { return m_center; }
	glm::vec2 point(std::size_t i) const { return m_pts[i]; }

	glm::vec2 normal(std::size_t first) const
	{
		std::size_t second = (first + 1) % m_pts.size();
		glm::vec2 perp{m_pts[first].y - m_pts[second].y, m_pts[second].x - m_pts[first].x};
		return glm::normalize(perp);
	}

	const auto &points() const { return m_pts; }
	auto pts_begin() const { return m_pts.begin(); }
	auto pts_end() const { return m_pts.end(); }

private:
	std::vector<glm::vec2> m_pts;
	glm::vec2 m_center;
};

class polygon_view
{
public:
	glm::vec2 offset;
	glm::vec2 scale;
	const polygon *poly;
private:
	float sin_angle;
	float cos_angle;

public:
	polygon_view(const polygon &_poly) : offset{}, poly{&_poly}, scale{1.f, 1.f}, sin_angle{0}, cos_angle{1} {}
	polygon_view(const polygon &_poly, glm::vec2 _offset, glm::vec2 _scale, float _angle) : offset{_offset}, poly{&_poly}, scale{_scale}, sin_angle{std::sin(_angle)}, cos_angle{std::cos(_angle)} {}

	void angle(float _angle)
	{
		sin_angle = std::sin(_angle);
		cos_angle = std::cos(_angle);
	}

	float angle() const
	{
		return std::atan2(sin_angle, cos_angle);
	}

	std::size_t size() const
	{
		return poly->size();
	}

	glm::vec2 point(std::size_t i) const
	{
		return transform(poly->point(i));
	}

	glm::vec2 normal(std::size_t i) const
	{
		glm::vec2 first = transform(poly->points()[i]);
		glm::vec2 second = transform(poly->points()[(i + 1) % poly->size()]);
		glm::vec2 perp{first.y - second.y, second.x - first.x};
		return glm::normalize(perp);
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
		glm::vec2 res{
			cos_angle * scale.x * pt.x - sin_angle * scale.y * pt.y + offset.x,
			sin_angle * scale.x * pt.x + cos_angle * scale.y * pt.y + offset.y
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
		res.push_back(rot_mat * glm::vec3(res.point(i - 1), 1));

	return res;
}

#endif