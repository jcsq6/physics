#ifndef WORLD_H
#define WORLD_H
#include <list>
#include <memory>

#include "constraint.h"

PHYSICS_BEG

void resolve_velocities(particle &p1, glm::vec2 p1_center, particle &p2, glm::vec2 p2_center, manifold collision_pt, glm::vec2 normal, float e);

class world
{
public:
	world(float world_width_meters, float world_height_meters, float gravity = -10);

	// make sure poly is not destroyed before world
	object *add_object(const polygon &poly, glm::vec2 pos, glm::vec2 v_init, float angle, float w_init, float mass, glm::vec2 scale);
	object *add_object(polygon &&, glm::vec2, glm::vec2, float, float, float, glm::vec2) = delete;

	// make sure poly is not destroyed before world
	// add object of infinite mass that stays in place
	object *add_static_object(const polygon &poly, glm::vec2 pos, float angle, float mass, glm::vec2 scale);
	object *add_static_object(polygon &&poly, glm::vec2 pos, float angle, glm::vec2 scale) = delete;

	void add_constraint(std::unique_ptr<constraint> c) { constraints.push_back(std::move(c)); }

	void update(float dt)
	{
		if (dt > time_step)
			for (; dt > 0; dt -= time_step)
				update_internal();
	}

private:
	struct collision_pair
	{
		std::list<object>::iterator a, b;
		glm::vec2 normal;
		manifold contact_pts;
	};

	std::list<object> objects;
	std::vector<collision_pair> collisions;
	std::vector<std::unique_ptr<constraint>> constraints;

	float grav;
	float world_width, world_height;

	static constexpr float time_step = .001f;

	void update_internal();
	void resolve_bounds();
};


PHYSICS_END


#endif