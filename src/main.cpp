#include "gl_object.h"
#include "bound.h"
#include "particle.h"
#include "world.h"

#include <chrono>
#include <string>
#include <iostream>

int main()
{
	constexpr int target_fps = 60;
	constexpr auto target_frame_duration = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::duration<double>(1.0 / target_fps));

	constexpr int window_width = 960;
	constexpr int window_height = 540;

	window win(window_width, window_height, "Physics");
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	shader program(
		"#version 330 core\n" // vertex shader
		"layout (location = 0) in vec2 pos;"
		"uniform mat4 ortho;"
		"uniform mat4 model;"
		"void main(){"
		"	gl_Position = ortho * model * vec4(pos, 0, 1);"
		"}", 

		"#version 330 core\n" // fragment shader
		"uniform vec4 color;"
		"out vec4 frag_color;"
		"void main(){\n"
		"	frag_color = color;"
		"}");

	vao my_vao;

	constexpr float world_width = window_width / world::pixels_per_meter;
	constexpr float world_height = window_height / world::pixels_per_meter;

	world handler(world_width, world_height, -20);

	auto triangle = regular_polygon(3);
	auto pentagon = regular_polygon(5);
	auto rect = regular_polygon(4);
	auto circle = regular_polygon(100);

	auto do_stacking_test = [&]()
	{
		handler.add_object(circle, {world_width / 2, 11}, {0, 0}, 0, 0, 10, {1, 1}, {1, 1, 0, 1});
		handler.add_object(pentagon, {world_width / 2, 9}, {0, 0}, 0, 0, 10, {1, 1}, {1, 0, 1, 1});
		handler.add_object(triangle, {world_width / 2, 7}, {0, 0}, 0, 0, 10, {.5, .5}, {1, 0, 0, 1});
		handler.add_object(rect, {world_width / 2, 5}, {0, 0}, 0, 0, 10, {1, 1}, {0, 0, 1, 1});
		handler.add_object(rect, {world_width / 2, 3}, {0, 0}, 0, 0, 10, {1, 1}, {0, 1, 0, 1});
	};
	
	auto do_velocity_test = [&]()
	{
		handler.add_object(circle, {12, 11}, {100, 0}, 0, 0, 10, {1, 1}, {1, 1, 0, 1});
		handler.add_object(pentagon, {18, 9}, {-1000, -1000}, 0, 0, 10, {1, 1}, {1, 0, 1, 1});
		handler.add_object(triangle, {2, 3}, {50, 50}, 0, 0, 10, {.5, .5}, {1, 0, 0, 1});
		handler.add_object(rect, {4, 12}, {-500, 0}, 0, 0, 10, {1, 1}, {0, 0, 1, 1});
		handler.add_object(rect, {22, 8}, {1000, 1000}, 0, 0, 10, {1, 1}, {0, 1, 0, 1});
	};

	// do_velocity_test();
	do_stacking_test();
	handler.add_object(triangle, {5, 6.5}, {20, 0}, 0, 0, 50, {2, 2}, {1, .5, .5, 1});
	handler.add_object(rect, {world_width / 2, 5}, {0, 0}, 0, 0, 10, {1, 3}, {1, 0, 0, 1});

	auto ortho = glm::ortho<float>(0, window_width, 0, window_height, -1.f, 1.f);

	while (!glfwWindowShouldClose(win.handle))
	{
		std::chrono::time_point frame_begin = std::chrono::steady_clock::now();

		glfwPollEvents();

		if (glfwGetKey(win.handle, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(win.handle, 1);

		if (glfwGetKey(win.handle, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			handler.update(1.f / target_fps / 2);
		else
			handler.update(1.f / target_fps);

		glClearColor(1, 1, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		glBindVertexArray(my_vao.id);

		glUseProgram(program.id);
		glUniformMatrix4fv(glGetUniformLocation(program.id, "ortho"), 1, GL_FALSE, &ortho[0][0]);

		handler.draw(0, glGetUniformLocation(program.id, "color"), glGetUniformLocation(program.id, "model"));

		glfwSwapBuffers(win.handle);

		auto frame_duration = std::chrono::steady_clock::now() - frame_begin;
		if (frame_duration < target_frame_duration)
		{
			auto sleep_time = target_frame_duration - frame_duration;
			auto start = std::chrono::steady_clock::now();
			while (std::chrono::steady_clock::now() - start < sleep_time);
		}
	}
}
