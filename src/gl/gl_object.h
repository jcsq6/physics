#ifndef GL_OBJECT_H
#define GL_OBJECT_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>

struct window
{
	window(int width, int height, const char *title)
	{
		glfwInit();

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		handle = glfwCreateWindow(width, height, title, nullptr, nullptr);
		glfwMakeContextCurrent(handle);

		glewInit();
	}

	~window()
	{
		glfwDestroyWindow(handle);
		glfwTerminate();
		handle = nullptr;
	}

	GLFWwindow *handle;
};

struct shader
{
	shader() : id{} {}
	shader(const char *vertex, const char *fragment) : id{}
	{
		GLuint vert_shader_id = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vert_shader_id, 1, &vertex, nullptr);
		glCompileShader(vert_shader_id);

		GLint completed = 0;
		glGetShaderiv(vert_shader_id, GL_COMPILE_STATUS, &completed);
		if (completed == GL_FALSE)
		{
			std::string message;
			int length = 0;
			glGetShaderiv(vert_shader_id, GL_INFO_LOG_LENGTH, &length);
			if (length)
			{
				message.resize(length);
				int stored_length;
				glGetShaderInfoLog(vert_shader_id, length, &stored_length, message.data());
			}

			std::cerr << "vertex shader compilation failed\n"
					  << message << '\n';
		}

		GLuint frag_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(frag_shader_id, 1, &fragment, nullptr);
		glCompileShader(frag_shader_id);

		completed = 0;
		glGetShaderiv(frag_shader_id, GL_COMPILE_STATUS, &completed);
		if (completed == GL_FALSE)
		{
			std::string message;
			int length = 0;
			glGetShaderiv(frag_shader_id, GL_INFO_LOG_LENGTH, &length);
			if (length)
			{
				message.resize(length);
				int stored_length;
				glGetShaderInfoLog(frag_shader_id, length, &stored_length, message.data());
			}

			std::cerr << "fragment shader compilation failed\n"
					  << message << '\n';
		}

		id = glCreateProgram();
		glAttachShader(id, vert_shader_id);
		glAttachShader(id, frag_shader_id);
		glLinkProgram(id);

		glDeleteShader(frag_shader_id);
		glDeleteShader(vert_shader_id);

		completed = 0;
		glGetProgramiv(id, GL_LINK_STATUS, &completed);
		if (completed == GL_FALSE)
		{
			std::string message;
			int length = 0;
			glGetProgramiv(id, GL_INFO_LOG_LENGTH, &length);
			if (length)
			{
				message.resize(length);
				int stored_length;
				glGetProgramInfoLog(id, length, &stored_length, message.data());
			}

			std::cerr << message << '\n';
		}
	}

	shader(const shader &) = delete;
	shader &operator=(const shader &) = delete;

	shader(shader &&other) : id{ other.id }
	{
		other.id = 0;
	}

	shader &operator=(shader &&other)
	{
		glDeleteProgram(id);
		id = other.id;
		other.id = 0;

		return *this;
	}

	~shader()
	{
		glDeleteProgram(id);
		id = 0;
	}

	GLuint id;
};

struct vao
{
	vao()
	{
		glGenVertexArrays(1, &id);
	}

	vao(const shader &) = delete;
	vao &operator=(const shader &) = delete;

	vao(vao &&other) : id{ other.id }
	{
		other.id = 0;
	}

	vao &operator=(vao &&other)
	{
		glDeleteVertexArrays(1, &id);
		id = other.id;
		other.id = 0;

		return *this;
	}

	~vao()
	{
		glDeleteVertexArrays(1, &id);
		id = 0;
	}

	GLuint id;
};

struct buffer
{
	buffer()
	{
		glGenBuffers(1, &id);
	}

	buffer(const buffer &) = delete;
	buffer &operator=(const buffer &) = delete;

	buffer(buffer &&other) : id{ other.id }
	{
		other.id = 0;
	}

	buffer &operator=(buffer &&other)
	{
		glDeleteBuffers(1, &id);
		id = other.id;
		other.id = 0;

		return *this;
	}

	~buffer()
	{
		glDeleteBuffers(1, &id);
		id = 0;
	}

	GLuint id;
};

using vbo = buffer;
using ebo = buffer;
using ubo = buffer;
using ssbo = buffer;

#endif