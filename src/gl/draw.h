#ifndef DRAW_H
#define DRAW_H

#include "draw_poly.h"
#include "bound.h"
#include "constraint.h"

#include <unordered_map>

static constexpr float pixels_per_meter = 40;

class drawer
{
public:
	void add_object(physics::object *obj, glm::vec4 color)
	{
		auto it = polys.find(obj->poly);
		if (it == polys.end())
			it = polys.emplace(obj->poly, obj->poly->points()).first;
		objects.push_back({obj, color, &it->second});
	}

	void draw(int pos_attribute, int color_uniform, int model_uniform) const
	{
		for (const auto &obj : objects)
		{
			auto model = glm::scale(glm::rotate(glm::translate(glm::mat4(1.f), {obj.obj->pt.pos * pixels_per_meter, 0}), obj.obj->pt.angle, {0, 0, 1}), {obj.obj->scale * pixels_per_meter, 0});
			glUniformMatrix4fv(model_uniform, 1, GL_FALSE, &model[0][0]);
			glUniform4fv(color_uniform, 1, &obj.color[0]);
			obj.drawable_shape->draw(pos_attribute);
		}
	}

private:
	struct draw_object
	{
		physics::object *obj;
		glm::vec4 color;
		const draw_poly *drawable_shape;
	};

	std::vector<draw_object> objects;
	std::unordered_map<const physics::polygon *, draw_poly> polys;
};


#endif