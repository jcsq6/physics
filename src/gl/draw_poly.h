#ifndef DRAW_POLY_H
#define DRAW_POLY_H
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include "gl_object.h"

class draw_poly
{
public:
	draw_poly(const std::vector<glm::vec2> &points) : m_size{points.size()}
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo.id);
		glBufferData(GL_ARRAY_BUFFER, m_size * sizeof(points[0]), points.data(), GL_STATIC_DRAW);
	}

	draw_poly(const draw_poly &) = delete;
	draw_poly &operator=(const draw_poly &) = delete;

	draw_poly(draw_poly &&) = default;
	draw_poly &operator=(draw_poly &&) = default;

	void draw(int pos_attribute) const
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo.id);
		glVertexAttribPointer(pos_attribute, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
		glEnableVertexAttribArray(pos_attribute);
		glDrawArrays(GL_TRIANGLE_FAN, 0, static_cast<GLsizei>(m_size));
	}

private:
	std::size_t m_size;
	vbo m_vbo;
};


#endif