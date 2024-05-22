#ifndef BOUND_H
#define BOUND_H

#define PHYSICS_BEG namespace physics {
#define PHYSICS_END }

#include <ostream>
#include <optional>
#include <vector>
#include <array>

#include <ranges>

#include <glm/gtc/matrix_transform.hpp>

#include "trig.h"

PHYSICS_BEG

inline glm::mat3 rot2d(float rads)
{
	auto c = std::cos(rads);
	auto s = std::sin(rads);
	glm::mat3 mat({c, s, 0}, {-s, c, 0}, {0, 0, 1});
	return mat;
}

consteval glm::mat3 const_rot2d(float rads)
{
	auto c = math::cos(rads);
	auto s = math::sin(rads);
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

class shape_view;

struct collision
{
	glm::vec2 normal;
	glm::vec2 a_contact; // contact point of shape "a" (first shape passed to collides)
	glm::vec2 b_contact; // contact point of shape "b" (second shape passed to collides)
	std::vector<glm::vec2> simplex;
	float dist;
	bool collides;

	collision() : normal{}, dist{}, collides{} {}
	collision(glm::vec2 _normal, float _dist, glm::vec2 a_pt, glm::vec2 b_pt) : normal{_normal}, a_contact{a_pt}, b_contact{b_pt}, dist{_dist}, collides{true} {}

	operator bool() const { return collides; }
};

using length_type = unsigned int;

// shapes are primative shapes, not meant to be transformed
// for transformed shapes, use shape_view's
class abstract_shape
{
public:
	static constexpr length_type infinity = std::numeric_limits<length_type>::max();

	virtual ~abstract_shape() = default;
	virtual glm::vec2 center() const { return {0, 0}; }
	virtual length_type size() const = 0;

protected:
	virtual glm::vec2 support(glm::vec2 dir) const = 0;

	friend collision collides(const shape_view &a, const shape_view &b);
	friend class shape_view;
};

constexpr static int dynamic_size = -1;

class abstract_polygon : public abstract_shape
{
public:
	virtual ~abstract_polygon() = default;
	virtual glm::vec2 point(length_type i) const = 0;
	virtual const glm::vec2 *data() const = 0;
protected:
	template <std::ranges::range R>
	static constexpr glm::vec2 poly_support(R &&pts, glm::vec2 dir)
	{
		glm::vec2 res{0, 0};
		float max = -std::numeric_limits<float>::infinity();
		for (auto pt : pts)
			if (auto d = glm::dot(pt, dir); d > max)
			{
				max = d;
				res = pt;
			}
		return res;
	}
};

template <length_type _size>
class regular_polygon : public abstract_polygon
{
public:
	constexpr regular_polygon()
	{
		constexpr float angle = 2 * glm::pi<float>() / _size;
		constexpr glm::mat3 rot_mat = const_rot2d(angle);
		m_pts[0] = {0, 1};

		// if even, rotate by half of the angle to make the bottom straight
		if constexpr (!(_size & 1))
			m_pts[0] = const_rot2d(angle / 2) * glm::vec3(m_pts[0], 1);

		for (length_type i = 1; i < _size; ++i)
			m_pts[i] = rot_mat * glm::vec3(m_pts[i - 1], 1);
	}

	constexpr length_type size() const override { return _size; }

	constexpr glm::vec2 point(length_type i) const override { return m_pts[i]; }
	constexpr const std::array<glm::vec2, _size> &points() const { return m_pts; }

	const glm::vec2 *data() const override { return m_pts.data(); }

private:
	std::array<glm::vec2, _size> m_pts;
protected:
	constexpr glm::vec2 support(glm::vec2 dir) const override { return poly_support(m_pts, dir); }
};

template <>
class regular_polygon<dynamic_size> : public abstract_polygon
{
public:
	regular_polygon() = default;
	regular_polygon(length_type size) { resize(size); }

	length_type size() const override { return static_cast<length_type>(m_pts.size()); }

	glm::vec2 point(length_type i) const override { return m_pts[i]; }
	const std::vector<glm::vec2> &points() const { return m_pts; }

	void resize(length_type new_size)
	{
		m_pts.resize(new_size);

		float angle = 2 * glm::pi<float>() / new_size;
		glm::mat3 rot_mat = rot2d(angle);

		m_pts[0] = {0, 1};
		
		// if even, rotate by half of the angle to make the bottom straight
		if (!(new_size & 1))
			m_pts[0] = rot2d(angle / 2) * glm::vec3(m_pts[0], 1);
		
		for (length_type i = 1; i < new_size; ++i)
			m_pts[i] = rot_mat * glm::vec3(m_pts[i - 1], 1);
	}

	const glm::vec2 *data() const override { return m_pts.data(); }
private:
	std::vector<glm::vec2> m_pts;
protected:
	constexpr glm::vec2 support(glm::vec2 dir) const override { return poly_support(m_pts, dir); }
};

template <length_type _size>
constexpr regular_polygon<_size> make_regular() { return regular_polygon<_size>(); }

inline regular_polygon<dynamic_size> make_regular(length_type size) { return regular_polygon<dynamic_size>(size); }

template <length_type _size>
class polygon : public abstract_polygon
{
public:
	static_assert(_size != 0);

	constexpr polygon() : m_center{0, 0} {}

	template <std::ranges::range R>
	constexpr polygon(R &&pts) { assign(std::forward<R>(pts)); }

	template <typename T>
	constexpr polygon(std::initializer_list<glm::vec<2, T>> pts) { assign(pts); }

	constexpr length_type size() const override { return _size; }
	constexpr glm::vec2 center() const override { return m_center; }

	constexpr glm::vec2 point(length_type i) const override { return m_pts[i]; }
	constexpr const std::array<glm::vec2, _size> &points() const { return m_pts; }

	template <std::ranges::range R>
	void assign(R &&pts)
	{
		m_center = {0, 0};
		std::size_t i = 0;
		for (auto pt : pts)
		{
			if (i >= _size)
				break;
			
			m_center += pt;
			m_pts[i++] = pt;
		}

		m_center /= _size;
	}

	const glm::vec2 *data() const override { return m_pts.data(); }
private:
	std::array<glm::vec2, _size> m_pts;
	glm::vec2 m_center;
protected:
	constexpr glm::vec2 support(glm::vec2 dir) const override { return poly_support(m_pts, dir); }
};

template <>
class polygon<dynamic_size> : public abstract_polygon
{
public:
	polygon() = default;

	template <typename T>
	polygon(std::initializer_list<glm::vec<2, T>> pts) : polygon() { assign(pts); }

	template <std::ranges::range R>
	polygon(R &&pts) : polygon() { assign(pts); }

	void reserve(std::size_t capacity) { m_pts.reserve(capacity); }

	void push_back(glm::vec2 pt)
	{
		m_pts.push_back(pt);
		m_center = (m_center * float(m_pts.size() - 1) + pt) / (float)m_pts.size();
	}

	template <std::ranges::sized_range R>
	void assign(R &&pts)
	{
		m_pts.clear();
		m_pts.reserve(std::ranges::size(pts));
		
		m_center = {0, 0};
		for (auto pt : pts)
		{
			m_pts.push_back(pt);
			m_center += pt;
		}

		if (m_pts.size())
			m_center /= m_pts.size();
	}

	length_type size() const override { return static_cast<length_type>(m_pts.size()); }
	glm::vec2 center() const override { return m_center; }

	glm::vec2 point(length_type i) const override { return m_pts[i]; }

	// glm::vec2 normal(std::size_t first) const
	// {
	// 	std::size_t second = (first + 1) % m_pts.size();
	// 	glm::vec2 perp{m_pts[first].y - m_pts[second].y, m_pts[second].x - m_pts[first].x};
	// 	return glm::normalize(perp);
	// }

	const std::vector<glm::vec2> &points() const { return m_pts; }

	const glm::vec2 *data() const override { return m_pts.data(); }

private:
	std::vector<glm::vec2> m_pts;
	glm::vec2 m_center;
protected:
	glm::vec2 support(glm::vec2 dir) const override { return poly_support(m_pts, dir); }
};

// radius 1
class circle : public abstract_shape
{
public:
	length_type size() const override { return infinity; }
protected:
	glm::vec2 support(glm::vec2 dir) const override { return dir; }
};

class shape_view
{
public:
	glm::vec2 offset;
	glm::vec2 scale;
	const abstract_shape *shape;
private:
	float sin_angle;
	float cos_angle;

public:
	shape_view(const abstract_shape &_shape) : offset{}, shape{&_shape}, scale{1.f, 1.f}, sin_angle{0}, cos_angle{1} {}
	shape_view(const abstract_shape &_shape, glm::vec2 _offset, glm::vec2 _scale, float _angle) : offset{_offset}, shape{&_shape}, scale{_scale}, sin_angle{std::sin(_angle)}, cos_angle{std::cos(_angle)} {}

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
		return shape->size();
	}

	// glm::vec2 point(std::size_t i) const
	// {
	// 	return transform(shape->point(i));
	// }

	// glm::vec2 normal(std::size_t i) const
	// {
	// 	glm::vec2 first = transform(shape->points()[i]);
	// 	glm::vec2 second = transform(shape->points()[(i + 1) % shape->size()]);
	// 	glm::vec2 perp{first.y - second.y, second.x - first.x};
	// 	return glm::normalize(perp);
	// }

	glm::vec2 center() const
	{
		// center doesn't change with rotation
		return shape->center() * scale + offset;
	}

	glm::vec2 transform(glm::vec2 pt) const
	{
		// as if 
		// translate = {{1, 0, 0}, {0, 1, 0}, {offset.x, offset.y, 1}};
		// scale = {{scale.x, 0, 0}, {0, scale.y, 0}, {0, 0, 1}};
		// rotate = {{std::cos(angle), std::sin(angle), 0}, {-std::sin(angle), std::cos(angle), 0}, {0, 0, 1}};
		// transform = translate * rotate * scale_mat;
		// return transform * vec3((*shape)[i].pt, 1);
		glm::vec2 res{
			cos_angle * scale.x * pt.x - sin_angle * scale.y * pt.y + offset.x,
			sin_angle * scale.x * pt.x + cos_angle * scale.y * pt.y + offset.y
		};

		return res;
	}

	glm::vec2 support(glm::vec2 dir) const
	{
		return transform(shape->support(dir));
	}
};

// returns collision with mtv to get a out of b or false if no collision
collision collides(const shape_view &a, const shape_view &b);
float moment_of_inertia(const shape_view &a);

PHYSICS_END

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

inline std::ostream &operator<<(std::ostream &stream, const physics::shape_view &p)
{
	if (auto poly = dynamic_cast<const physics::abstract_polygon *>(p.shape))
	{
		stream << "Polygon(";
		if (p.size())
		{
			stream << p.transform(poly->point(0));
			for (physics::length_type i = 1; i < p.size(); ++i)
				stream << ", " << p.transform(poly->point(i));
		}
		stream << ')';
	}

	return stream;
}

#endif