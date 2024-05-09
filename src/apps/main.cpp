#include "gl_object.h"
#include "bound.h"
#include "particle.h"
#include "world.h"

#include <chrono>
#include <string>
#include <iostream>

void mouse_pos_interp(glm::dvec2 window_min, glm::dvec2 window_max, glm::dvec2 target_min, glm::dvec2 target_max, glm::dvec2 &mouse_pos)
{
	// y = (window_size)
	mouse_pos.x = (target_max.x - target_min.x) / (window_max.x - window_min.x) * (mouse_pos.x - window_min.x) + target_min.x;
	double y = window_max.y - mouse_pos.y;
	mouse_pos.y = (target_max.y - target_min.y) / (window_max.y - window_min.y) * (y - window_min.y) + target_min.y;
}

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

	world handler(world_width, world_height, -25);

	auto triangle = std::make_shared<polygon>(regular_polygon(3));
	auto pentagon = std::make_shared<polygon>(regular_polygon(5));
	auto rect = std::make_shared<polygon>(regular_polygon(4));
	auto circle = std::make_shared<polygon>(regular_polygon(100));
	auto hexagon = std::make_shared<polygon>(regular_polygon(6));

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
	// do_stacking_test();
	handler.add_object(rect, {world_width / 2, 5}, {0, 0}, 0, 0, 10, {1, 3}, {1, 0, 0, 1});
	handler.add_object(hexagon, {12, 10}, {-20, 0}, 0.f, 0.f, 10.f, {2, 2}, {1, 1, .5, 1});
	auto triangle_object = handler.add_object(triangle, {5, 6.5}, {100, 0}, 0, 0, 50, {2, 2}, {1, .5, .5, 1});

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

		// glm::dvec2 mouse;
		// glfwGetCursorPos(win.handle, &mouse.x, &mouse.y);
		// mouse_pos_interp({0, 0}, {window_width, window_height}, {0, 0}, {window_width, window_height}, mouse);

		// mouse /= world::pixels_per_meter;

		// std::cout << mouse << ", " << triangle_object->pt.pos << '\n';

		// polygon_view cursor(rect, mouse, {1 / world::pixels_per_meter, 1 / world::pixels_per_meter}, 0.f);
		// polygon_view tobj_view(*triangle_object->poly->first, triangle_object->pt.pos, triangle_object->scale, triangle_object->pt.angle);
		// if (collides(cursor, tobj_view))
		// {
		// 	std::cout << "hi\n";
		// 	auto old_pt = triangle_object->pt.pos;
		// 	triangle_object->pt.pos = mouse;

		// 	triangle_object->pt.v = (triangle_object->pt.pos - old_pt) * (float)target_fps;
		// 	triangle_object->pt.w = 0;
		// }

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
