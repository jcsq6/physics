#ifndef DRAW_H
#define DRAW_H

#include "bound.h"
#include "object.h"

#include <unordered_map>
#include <memory>
#include <ranges>
#include <span>

#include "gl_object.h"

class gl_instance
{
public:
	gl_instance(int width, int height, glm::vec2 target_min, glm::vec2 target_dims, const char *title);

	gl_instance(const gl_instance &) = delete;
	gl_instance& operator=(const gl_instance &) = delete;

	static void framebuffer_size_callback(GLFWwindow* window, int width, int height);

	const window &get_window() const { return m_window; }
	const glm::mat4 &get_ortho() const { return m_ortho; }

	const shader &get_shape_program() const { return m_shape_program; }
	const shader &get_circle_program() const { return m_circle_program; }

	const vao &get_square_vao() const { return m_square_vao; }

	glm::dvec2 get_mouse_pos() const;

	// adjusted for dpi
	glm::ivec2 viewport_min() const { return m_min; }
	// adjusted for dpi
	glm::ivec2 viewport_size() const { return m_size; }

	glm::vec2 target_min() const { return m_target_min; }
	glm::vec2 target_size() const { return m_target_size; }

private:
	struct glfw_instance
	{
		glfw_instance() { glfwInit(); }
		~glfw_instance() { glfwTerminate(); }
	};

	glfw_instance m_glfw;
	window m_window;
	shader m_shape_program;
	shader m_circle_program;
	glm::mat4 m_ortho;
	vao m_square_vao;
	vbo m_square_vbo;
	glm::ivec2 m_min;
	glm::ivec2 m_size;
	glm::vec2 m_target_min;
	glm::vec2 m_target_size;
};

class draw_shape
{
public:
	virtual void draw(gl_instance &gl, glm::vec4 color, glm::vec2 pos, glm::vec2 scale, float angle) const = 0;
	virtual ~draw_shape() = default;
};

class draw_poly : public draw_shape
{
public:
	template <std::ranges::contiguous_range R>
	draw_poly(R &&points) : m_size{std::ranges::size(points)}
	{
		init(std::ranges::data(points));
	}

	draw_poly(const draw_poly &) = delete;
	draw_poly &operator=(const draw_poly &) = delete;

	draw_poly(draw_poly &&) = default;
	draw_poly &operator=(draw_poly &&) = default;

	void draw(gl_instance &gl, glm::vec4 color, glm::vec2 pos, glm::vec2 scale, float angle) const override;

private:
	std::size_t m_size;
	vao m_vao;
	vbo m_vbo;

	void init(const glm::vec2 *data);
};

// radius 1, so the scale passed to draw is the radius
class draw_circle : public draw_shape
{
public:
	draw_circle() = default;

	void draw(gl_instance &gl, glm::vec4 color, glm::vec2 pos, glm::vec2 scale, float angle) const override;
};


inline std::unique_ptr<draw_shape> make_shape(const physics::abstract_shape &shape)
{
	if (auto shape_poly = dynamic_cast<const physics::abstract_polygon *>(&shape))
		return std::make_unique<draw_poly>(std::span<const glm::vec2>(shape_poly->data(), shape_poly->size()));
	else if (auto shape_circle = dynamic_cast<const physics::circle *>(&shape))
		return std::make_unique<draw_circle>();
	else
		return {};
}

class drawer
{
public:
	void add_object(physics::object *obj, glm::vec4 color)
	{
		auto it = shapes.find(obj->shape);
		if (it == shapes.end())
			it = shapes.emplace(obj->shape, make_shape(*obj->shape)).first;
		objects.push_back({obj, color, it->second.get()});
	}

	void draw(gl_instance &gl) const
	{
		for (const auto &obj : objects)
			obj.drawable_shape->draw(gl, obj.color, obj.obj->pt.pos, obj.obj->scale, obj.obj->pt.angle);
	}

private:
	struct draw_object
	{
		physics::object *obj;
		glm::vec4 color;
		const draw_shape *drawable_shape;
	};

	std::vector<draw_object> objects;
	std::unordered_map<const physics::abstract_shape *, std::unique_ptr<draw_shape>> shapes;
};

#endif