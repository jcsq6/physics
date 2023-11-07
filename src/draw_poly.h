#ifndef DRAW_POLY_H
#define DRAW_POLY_H
#include <vector>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include "gl_object.h"

class draw_poly
{
public:
	template <typename It>
	draw_poly(It begin, It end) : m_pts(begin, end)
	{
		update_buffers();
	}

	draw_poly(std::vector<glm::vec2> &&points) : m_pts(std::move(points))
	{
		update_buffers();
	}

	draw_poly(const std::vector<glm::vec2> &points) : m_pts(points)
	{
		update_buffers();
	}

	draw_poly(const draw_poly &) = delete;
	draw_poly &operator=(const draw_poly &) = delete;

	void draw(int pos_attribute) const
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo.id);
		glVertexAttribPointer(pos_attribute, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
		glEnableVertexAttribArray(pos_attribute);
		glDrawArrays(GL_TRIANGLE_FAN, 0, static_cast<GLsizei>(m_pts.size()));
	}

private:
	std::vector<glm::vec2> m_pts;
	vbo m_vbo;

	void update_buffers()
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo.id);
		glBufferData(GL_ARRAY_BUFFER, m_pts.size() * sizeof(m_pts[0]), m_pts.data(), GL_STATIC_DRAW);
	}
};


#endif