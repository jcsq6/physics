#include "world.h"
#include "draw.h"

#include <chrono>
#include <string>

#include "json.hpp"

// #include <iostream>

void mouse_pos_interp(glm::dvec2 window_min, glm::dvec2 window_max, glm::dvec2 target_min, glm::dvec2 target_max, glm::dvec2 &mouse_pos)
{
	// y = (window_size)
	mouse_pos.x = (target_max.x - target_min.x) / (window_max.x - window_min.x) * (mouse_pos.x - window_min.x) + target_min.x;
	double y = window_max.y - mouse_pos.y;
	mouse_pos.y = (target_max.y - target_min.y) / (window_max.y - window_min.y) * (y - window_min.y) + target_min.y;
}

physics::world parse_json(const char *filename, drawer &world_drawer);

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		std::cerr << "Usage: " << argv[0] << " <config_file.json>\n";
		std::exit(1);
	}

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

	drawer world_drawer;
	physics::world handler;

	try
	{
		handler = parse_json(argv[1], world_drawer);
	}
	catch(const std::exception& e)
	{
		std::cerr << "Failed to read config file: " << e.what() << '\n';
		std::exit(1);
	}

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

		glClearColor(1, 1, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		glBindVertexArray(my_vao.id);

		glUseProgram(program.id);
		glUniformMatrix4fv(glGetUniformLocation(program.id, "ortho"), 1, GL_FALSE, &ortho[0][0]);

		world_drawer.draw(0, glGetUniformLocation(program.id, "color"), glGetUniformLocation(program.id, "model"));

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

#include <unordered_map>
#include <fstream>

physics::world parse_json(const char *filename, drawer &world_drawer)
{
	std::ifstream file(filename);
	if (!file)
	{
		std::cerr << "Could not open file: " << filename << '\n';
		std::exit(1);
	}

	nlohmann::json data = nlohmann::json::parse(file);
	float gravity;
	if (data.contains("gravity"))
		gravity = data["gravity"];
	else
		gravity = -25;
	if (!data.contains("width"))
	{
		std::cerr << "Could not find width in config file\n";
		std::exit(1);
	}
	if (!data.contains("height"))
	{
		std::cerr << "Could not find height in config file\n";
		std::exit(1);
	}

	physics::world res(data["width"], data["height"], gravity);

	static auto triangle = physics::regular_polygon(3);
	static auto pentagon = physics::regular_polygon(5);
	static auto rect = physics::regular_polygon(4);
	static auto circle = physics::regular_polygon(100);
	static auto hexagon = physics::regular_polygon(6);

	std::unordered_map<std::string, physics::object *> objects;

	if (data.contains("objects"))
	{
		int i = 0;
		for (auto &o : data["objects"])
		{
			++i;
			std::unordered_map<std::string, physics::object *>::iterator loc;
			if (o.contains("name"))
			{
				if (objects.find(o["name"]) != objects.end())
				{
					std::cerr << "Duplicate object name: " << o["name"] << std::endl;
					continue;
				}

				loc = objects.insert({o["name"], nullptr}).first;
			}
			else
			{
				std::cerr << "No name in object #" << i << std::endl;
				continue;
			}

			physics::polygon *p = nullptr;

			if (o.contains("shape"))
			{
				if (o["shape"] == "triangle")
					p = &triangle;
				else if (o["shape"] == "pentagon")
					p = &pentagon;
				else if (o["shape"] == "rectangle")
					p = &rect;
				else if (o["shape"] == "circle")
					p = &circle;
				else if (o["shape"] == "hexagon")
					p = &hexagon;
			}
			else
			{
				std::cerr << "No shape in object #" << i << std::endl;
				continue;
			}

			glm::vec2 pos;
			if (o.contains("pos"))
				pos = {o["pos"][0], o["pos"][1]};
			else
			{
				std::cerr << "No pos in object #" << i << std::endl;
				continue;
			}

			glm::vec2 scale;
			if (o.contains("scale"))
				scale = {o["scale"][0], o["scale"][1]};
			else
			{
				std::cerr << "No scale in object #" << i << std::endl;
				continue;
			}

			float angle;
			if (o.contains("angle"))
				angle = o["angle"];
			else
				angle = 0;

			glm::vec4 color;
			if (o.contains("color"))
				color = {o["color"][0], o["color"][1], o["color"][2], o["color"][3]};
			else
				color = {0, 0, 0, 1};

			enum class object_type { dynamic, static_type };
			object_type type;
			if (o.contains("type"))
			{
				if (o["type"] == "dynamic")
					type = object_type::dynamic;
				else if (o["type"] == "static")
					type = object_type::static_type;
				else
				{
					std::cerr << "Unknown object type: " << o["type"] << std::endl;
					continue;
				}
			}
			else
			{
				std::cerr << "No type in object #" << i << std::endl;
				continue;
			}

			if (type == object_type::dynamic)
			{
				glm::vec2 vel;
				if (o.contains("vel"))
					vel = {o["vel"][0], o["vel"][1]};
				else
					vel = {};

				float w;
				if (o.contains("w"))
					w = o["w"];
				else
					w = 0;

				float mass;
				if (o.contains("mass"))
					mass = o["mass"];
				else
					mass = 1;

				loc->second = res.add_object(*p, pos, vel, angle, w, mass, scale);
			}
			else
				loc->second = res.add_static_object(*p, pos, angle, scale);

			world_drawer.add_object(loc->second, color);
		}
	}

	if (data.contains("constraints"))
	{
		int i = 0;
		for (auto &c : data["constraints"])
		{
			++i;
			if (c.contains("type"))
			{
				if (c["type"] == "position")
				{
					if (c.contains("objects"))
					{
						if (c["objects"].size() != 2)
						{
							std::cerr << "Position constraint must have two objects" << std::endl;
							continue;
						}

						auto obj1 = objects.find(c["objects"][0]);
						auto obj2 = objects.find(c["objects"][1]);

						if (obj1 == objects.end())
						{
							std::cerr << "Object " << c["objects"][0] << " not found" << std::endl;
							continue;
						}
						if (obj2 == objects.end())
						{
							std::cerr << "Object " << c["objects"][1] << " not found" << std::endl;
							continue;
						}

						float dist;
						if (c.contains("distance"))
							dist = c["distance"];
						else
						{
							std::cerr << "No distance in constraint #" << i << std::endl;
							continue;
						}

						res.add_constraint(std::make_unique<physics::position_constraint>(*obj1->second, *obj2->second, dist));
					}
					else
					{
						std::cerr << "No objects in constraint #" << i << std::endl;
						continue;
					}
				}
				else
				{
					std::cerr << "Unknown constraint type: " << c["type"] << std::endl;
					continue;
				}
			}
			else
			{
				std::cerr << "No type in constraint #" << i << std::endl;
				continue;
			}
		}
	}

	return res;
}