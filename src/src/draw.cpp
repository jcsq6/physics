#include "draw.h"
#include <numeric>

static constexpr const char *shape_vert =
	"#version 330 core\n" // vertex shader
	"layout (location = 0) in vec2 pos;"
	"uniform mat4 model_view;" // ortho * model
	"void main(){"
	"	gl_Position = model_view * vec4(pos, 0, 1);"
	"}";

static constexpr const char *shape_frag =
	"#version 330 core\n" // fragment shader
	"uniform vec4 color;"
	"out vec4 frag_color;"
	"void main(){\n"
	"	frag_color = color;"
	"}";

static constexpr const char *circle_vert =
	"#version 330 core\n" // vertex shader
	"layout (location = 0) in vec2 pos;"
	"uniform mat4 model_view;" // ortho * model
	"out vec4 frag_pos;"
	"void main(){"
	"	gl_Position = model_view * vec4(pos, 0, 1);"
	"   frag_pos = gl_Position;"
	"}";
static constexpr const char *circle_frag = 
	"#version 330 core\n"
	"uniform vec2 trans_center;" // ortho * model * vec4(0, 0, 0, 1)
	"uniform vec2 trans_radius;" // radius / target_size * 2
	"uniform vec4 color;"
	"in vec4 frag_pos;"
	"out vec4 frag_color;"
	"void main(){"
	"	vec2 normalized = (frag_pos.xy - trans_center) / trans_radius;"
	"	float dist = length(normalized);"
	"	if (dist >= 1.0) {"
	"		discard;"
	"	} else {"
	"		frag_color = color;"
	"	}"
	"}";

static constexpr int pos_attribute = 0;

gl_instance::gl_instance(int width, int height, glm::vec2 target_min, glm::vec2 target_dims, const char *title) :
	m_glfw(), m_window(width, height, title),
	m_shape_program(shape_vert, shape_frag), m_circle_program(circle_vert, circle_frag),
	m_ortho(glm::ortho<float>(target_min.x, target_min.x + target_dims.x, target_min.y, target_min.y + target_dims.y, -1, 1)),
	m_min{}, m_size{width, height},
	m_target_min{target_min}, m_target_size{target_dims}
{
	static const glm::vec2 square_pts[] = {{-1.f, -1.f}, {1.f, -1.f}, {1.f, 1.f}, {-1.f, 1.f}};

	glfwSetWindowUserPointer(m_window.handle, this);
	glfwSetFramebufferSizeCallback(m_window.handle, framebuffer_size_callback);

	// setup square vao and vbo
	glBindVertexArray(m_square_vao.id);
	glBindBuffer(GL_ARRAY_BUFFER, m_square_vbo.id);
	
	glBufferData(GL_ARRAY_BUFFER, sizeof(square_pts), square_pts, GL_STATIC_DRAW);
	glVertexAttribPointer(pos_attribute, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);

	glEnableVertexAttribArray(pos_attribute);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void mouse_pos_interp(glm::dvec2 window_min, glm::dvec2 window_max, glm::dvec2 target_min, glm::dvec2 target_max, glm::dvec2 &mouse_pos)
{
	mouse_pos.x = (target_max.x - target_min.x) / (window_max.x - window_min.x) * (mouse_pos.x - window_min.x) + target_min.x;
	double size = window_max.y - window_min.y;
	mouse_pos.y = (target_max.y - target_min.y) / (size) * (size - mouse_pos.y) + target_min.y;
}

glm::dvec2 gl_instance::get_mouse_pos() const
{
	glm::dvec2 mouse_pos;
	glfwGetCursorPos(m_window.handle, &mouse_pos.x, &mouse_pos.y);

	mouse_pos_interp({m_min.x, m_min.y}, {m_min.x + m_size.x, m_min.y + m_size.y}, {m_target_min.x, m_target_min.y}, {m_target_min.x + m_target_size.x, m_target_min.y + m_target_size.y}, mouse_pos);

	return mouse_pos;
}

void draw_poly::draw(gl_instance &gl, glm::vec4 color, glm::vec2 pos, glm::vec2 scale, float angle) const
{
	const auto &program = gl.get_shape_program();

	glUseProgram(program.id);

	auto model = glm::scale(glm::rotate(glm::translate(glm::mat4(1.f), {pos, 0}), angle, {0, 0, 1}), {scale, 0});
	auto model_view = gl.get_ortho() * model;
	glUniformMatrix4fv(glGetUniformLocation(program.id, "model_view"), 1, GL_FALSE, &model_view[0][0]);
	glUniform4fv(glGetUniformLocation(program.id, "color"), 1, &color[0]);

	glBindVertexArray(m_vao.id);
	glDrawArrays(GL_TRIANGLE_FAN, 0, static_cast<GLsizei>(m_size));
}

void draw_poly::init(const glm::vec2 *data)
{
	glBindVertexArray(m_vao.id);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo.id);
	
	glBufferData(GL_ARRAY_BUFFER, m_size * sizeof(glm::vec2), data, GL_STATIC_DRAW);
	glVertexAttribPointer(pos_attribute, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);

	glEnableVertexAttribArray(pos_attribute);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_circle::draw(gl_instance &gl, glm::vec4 color, glm::vec2 pos, glm::vec2 scale, float angle) const
{
	const auto &program = gl.get_circle_program();

	glUseProgram(program.id);

	auto model = glm::scale(glm::rotate(glm::translate(glm::mat4(1.f), {pos, 0}), angle, {0, 0, 1}), {scale, 0});
	auto model_view = gl.get_ortho() * model;
	glm::vec2 trans_center(model_view * glm::vec4(0, 0, 0, 1));
	glm::vec2 trans_radius = scale / glm::vec2(gl.target_size()) * 2.f;

	glUniformMatrix4fv(glGetUniformLocation(program.id, "model_view"), 1, GL_FALSE, &model_view[0][0]);
	glUniform4fv(glGetUniformLocation(program.id, "color"), 1, &color[0]);
	glUniform2fv(glGetUniformLocation(program.id, "trans_center"), 1, &trans_center[0]);
	glUniform2fv(glGetUniformLocation(program.id, "trans_radius"), 1, &trans_radius[0]);

	glBindVertexArray(gl.get_square_vao().id);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void gl_instance::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	gl_instance *owner = static_cast<gl_instance *>(glfwGetWindowUserPointer(window));

	constexpr int scale_factor = 100000;
	int scaled_width = static_cast<int>(owner->m_target_size.x * scale_factor);
	int scaled_height = static_cast<int>(owner->m_target_size.y * scale_factor);
	int gcd = std::gcd(scaled_width, scaled_height);
	int aspect_ratio_x = scaled_width / gcd;
	int aspect_ratio_y = scaled_height / gcd;

	int leftover_width = width % aspect_ratio_x;
	int leftover_height = height % aspect_ratio_y;

	int new_width = width - leftover_width;
	int new_height = height - leftover_height;

	int height_from_keep_x = new_width * aspect_ratio_y / aspect_ratio_x;
	int width_from_keep_y = height * aspect_ratio_x / aspect_ratio_y;

	int diff_x = new_width - width_from_keep_y;
	int diff_y = new_height - height_from_keep_x;

	// keep height
	if (diff_y < diff_x)
	{
		new_width = width_from_keep_y;
		leftover_width = width - new_width;
	}
	// keep width
	else
	{
		new_height = height_from_keep_x;
		leftover_height = height - new_height;
	}

    glViewport(leftover_width / 2, leftover_height / 2, new_width, new_height);

	// if (owner->m_draw)
	// 	owner->m_draw();
	
	float xscale, yscale;
	glfwGetWindowContentScale(window, &xscale, &yscale);

	owner->m_min.x = static_cast<int>(leftover_width / xscale / 2);
	owner->m_min.y = static_cast<int>(leftover_height / yscale / 2);
	owner->m_size.x = static_cast<int>(new_width / xscale);
	owner->m_size.y = static_cast<int>(new_height / yscale);
}