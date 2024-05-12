#include "world.h"
#include "draw.h"

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
	// polygon A = {glm::vec2{6, 4}, {2, 8}, {5, 11}, {9, 7}};
	// polygon B = {glm::vec2{4, 2}, {4, 5}, {12, 5}, {12, 2}};

	// // auto t = regular_polygon(3);
	// // t.orient();

	// // polygon A = {glm::vec2{13, 3}, {14, 7}, {10, 8}, {9, 4}};
	// // polygon B = {glm::vec2{12, 2}, {12, 5}, {4, 5}, {4, 2}};

	// // polygon A = {glm::vec2{8, 4}, {8, 9}, {14, 9}, {14, 4}};
	// // polygon B = {glm::vec2{12, 2}, {12, 5}, {4, 5}, {4, 2}};

	// polygon_view A_view(A);
	// polygon_view B_view(B);

	// // polygon A = regular_polygon(3);
	// // polygon B = regular_polygon(4);

	// // polygon_view A_view(A, {2, 2}, {3, 3}, 0);
	// // polygon_view B_view(B, {6, 3}, {4, 2}, 0);

	// collision coll = collides(A_view, B_view);
	// if (coll)
	// {
	// 	manifold intersects = contact_manifold(A_view, B_view, coll);
	// 	for (unsigned int i = 0; i < 2; ++i)
	// 		if (intersects.pts[i])
	// 			std::cout << '(' << intersects.pts[i]->x << ", " << intersects.pts[i]->y << ")\n";
	// }

	// return 0;

	constexpr int target_fps = 60;
	constexpr auto target_frame_duration = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::duration<double>(1.0 / target_fps));

	constexpr int window_width = 500;
	constexpr int window_height = 500;

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

	constexpr float world_width = window_width / pixels_per_meter;
	constexpr float world_height = window_height / pixels_per_meter;

	// world handler(world_width, world_height, -20);

	auto triangle = physics::regular_polygon(3);
	// auto pentagon = regular_polygon(5);
	auto rect = physics::regular_polygon(4);
	// auto circle = regular_polygon(100);

	physics::polygon rect_ccw = {glm::vec2{0, 0}, {1, 0}, {1, 1}, {0, 1}};

	physics::polygon_view a_view(rect, {0, 0}, {2, 1}, 0);
	physics::polygon_view b_view(rect_ccw, {0, 1.25}, {1, .5}, 0);
	// b_view.angle = -.314159;
	// b_view.offset = {.690516,.0630938};

	draw_poly a_drawable(rect.points());
	draw_poly b_drawable(rect_ccw.points());
	draw_poly arrow({glm::vec2{-.2f, 0.f}, {-.2f, .5f}, {-.5f, .5f}, {0.f, 1.f}, {.5f, .5f}, {.2f, .5f}, {.2, 0}});

	draw_poly circle(physics::regular_polygon_pts(100));

	auto ortho = glm::ortho<float>(-3, 3, -3, 3, -1.f, 1.f);

	while (!glfwWindowShouldClose(win.handle))
	{
		std::chrono::time_point frame_begin = std::chrono::steady_clock::now();

		glfwPollEvents();

		if (glfwGetKey(win.handle, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(win.handle, 1);

		if (glfwGetKey(win.handle, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)
			b_view.angle(b_view.angle() + glm::radians(45.f) / target_fps);

		if (glfwGetKey(win.handle, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			b_view.angle(b_view.angle() - glm::radians(45.f) / target_fps);
		
		if (glfwGetKey(win.handle, GLFW_KEY_ENTER) == GLFW_PRESS)
		{
			// std::cout << "A = " << a_view << '\n';
			// std::cout << "B = " << b_view << '\n';
			std::cout << "angle: " << b_view.angle() << '\n';
			std::cout << "offset: " << b_view.offset << '\n';
			std::cout << "scale: " << b_view.scale << '\n';
		}
		
		glm::dvec2 pos;
		glfwGetCursorPos(win.handle, &pos.x, &pos.y);
		mouse_pos_interp({0, 0}, {window_width, window_height}, {-3, -3}, {3, 3}, pos);
		b_view.offset = pos;

		// b_view.offset = a_view.offset + glm::vec2{1, -.5};

		auto res = collides(a_view, b_view);

		// if (glfwGetKey(win.handle, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		// 	handler.update(1.f / target_fps / 2);
		// else
		// 	handler.update(1.f / target_fps);

		glClearColor(1, 1, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		glBindVertexArray(my_vao.id);

		glUseProgram(program.id);
		glUniformMatrix4fv(glGetUniformLocation(program.id, "ortho"), 1, GL_FALSE, &ortho[0][0]);

		auto model = glm::scale(glm::rotate(glm::translate(glm::mat4(1.f), {b_view.offset, 0}), b_view.angle(), {0, 0, 1}), {b_view.scale, 0});
		auto color = res ? glm::vec4{0, 1, 0, .75} : glm::vec4{1, 0, 0, .75};
		glUniformMatrix4fv(glGetUniformLocation(program.id, "model"), 1, GL_FALSE, &model[0][0]);
		glUniform4fv(glGetUniformLocation(program.id, "color"), 1, &color[0]);
		b_drawable.draw(0);

		model = glm::scale(glm::rotate(glm::translate(glm::mat4(1.f), {a_view.offset, 0}), a_view.angle(), {0, 0, 1}), {a_view.scale, 0});
		color = {1, 1, 0, .75};
		glUniformMatrix4fv(glGetUniformLocation(program.id, "model"), 1, GL_FALSE, &model[0][0]);
		glUniform4fv(glGetUniformLocation(program.id, "color"), 1, &color[0]);
		a_drawable.draw(0);

		if (res)
		{
			physics::manifold contacts = contact_manifold(a_view, b_view, res);
			color = {0, 0, 0, 1};
			glUniform4fv(glGetUniformLocation(program.id, "color"), 1, &color[0]);
			for (std::size_t i = 0; i < 2; ++i)
			{
				if (contacts.pts[i])
				{
					model = glm::scale(glm::translate(glm::mat4(1.f), {*contacts.pts[i], 0}), {.03, .03, 0});
					glUniformMatrix4fv(glGetUniformLocation(program.id, "model"), 1, GL_FALSE, &model[0][0]);
					circle.draw(0);
				}
			}
		}

		// handler.draw(0, glGetUniformLocation(program.id, "color"), glGetUniformLocation(program.id, "model"));

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
