#include "bound.h"
#include "draw.h"

#include <chrono>
#include <string>
#include <iostream>

int main()
{
	constexpr int target_fps = 60;
	constexpr auto target_frame_duration = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::duration<double>(1.0 / target_fps));

	constexpr int window_width = 500;
	constexpr int window_height = 500;

	gl_instance gl(window_width, window_height, glm::vec2(-3, -3), glm::vec2(6, 6), "Physics");

	const window &win = gl.get_window();
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	auto triangle = physics::make_regular<3>();
	// auto pentagon = regular_polygon(5);
	auto rect = physics::make_regular<4>();
	// auto circle = regular_polygon(100);

	physics::shape_view a_view(rect, {0, 0}, {1, 1}, 0);
	physics::shape_view b_view(triangle, {-1, 0}, {1, 1}, 0);

	draw_poly a_drawable(rect.points());
	draw_poly b_drawable(triangle.points());
	// draw_poly arrow({glm::vec2{-.2f, 0.f}, {-.2f, .5f}, {-.5f, .5f}, {0.f, 1.f}, {.5f, .5f}, {.2f, .5f}, {.2, 0}});

	draw_circle circle;

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
			std::cout << "A = " << a_view << '\n';
			std::cout << "B = " << b_view << '\n';
			std::cout << "angle: " << b_view.angle() << '\n';
			std::cout << "offset: " << b_view.offset << '\n';
			std::cout << "scale: " << b_view.scale << '\n';
		}
		
		b_view.offset = gl.get_mouse_pos();

		auto res = collides(a_view, b_view);

		glClearColor(1, 1, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		b_drawable.draw(gl, res ? glm::vec4{0, 1, 0, .75} : glm::vec4{1, 0, 0, .75}, b_view.offset, b_view.scale, b_view.angle());
		a_drawable.draw(gl, {1, 1, 0, .75}, a_view.offset, a_view.scale, a_view.angle());

		if (res)
			for (auto pt : {res.a_contact, res.b_contact})
				circle.draw(gl, {0, 0, 1, 1}, pt, {.03, .03}, 0);

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
