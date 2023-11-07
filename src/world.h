#ifndef WORLD_H
#define WORLD_H
#include <unordered_map>
#include <algorithm>
#include <memory>
#include "bound.h"
#include "particle.h"
#include "draw_poly.h"

void resolve_velocities(particle &p1, glm::vec2 p1_center, particle &p2, glm::vec2 p2_center, glm::vec2 collision_pt, glm::vec2 normal, float e);

struct object
{
	particle pt;
	glm::vec4 color;
	glm::vec2 scale;
	std::unordered_map<const polygon *, std::unique_ptr<draw_poly>>::const_iterator poly;
};

class world
{
public:
	static constexpr float pixels_per_meter = 40;

	world(float world_width_meters, float world_height_meters, float gravity = -10);

	void add_object(const polygon &poly, glm::vec2 pos, glm::vec2 v_init, float angle, float w_init, float mass, glm::vec2 scale, const glm::vec4 &color);
	void add_object(polygon &&poly, glm::vec2 pos, glm::vec2 v_init, float angle, float w_init, float mass, glm::vec2 scale, const glm::vec4 &color) = delete;

	void update(float dt)
	{
		if (dt > time_step)
			for (; dt > 0; dt -= time_step)
				update_internal();
	}

	void draw(int pos_attribute, int color_uniform, int model_uniform) const
	{
		for (const auto &obj : objects)
		{
			if (!obj.poly->second)
				continue;
			
			auto model = glm::scale(glm::rotate(glm::translate(glm::mat4(1.f), {obj.pt.pos * pixels_per_meter, 0}), obj.pt.angle, {0, 0, 1}), {obj.scale * pixels_per_meter, 0});
			glUniformMatrix4fv(model_uniform, 1, GL_FALSE, &model[0][0]);
			glUniform4fv(color_uniform, 1, &obj.color[0]);
			obj.poly->second->draw(pos_attribute);
		}
	}
private:
	struct collision_pair
	{
		std::vector<object>::iterator a, b;
		glm::vec2 normal;
		manifold contact_pts;
	};

	std::vector<object> objects;
	std::vector<collision_pair> collisions;
	std::unordered_map<const polygon *, std::unique_ptr<draw_poly>> shapes;
	float grav;
	float world_width, world_height;

	static constexpr float time_step = .001f;

	void update_internal();
	void resolve_bounds();
};


#endif