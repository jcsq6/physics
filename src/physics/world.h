#ifndef WORLD_H
#define WORLD_H
#include <list>
#include <memory>

#include "constraint.h"

PHYSICS_BEG

void resolve_velocities(particle &p1, glm::vec2 p1_center, particle &p2, glm::vec2 p2_center, const collision &coll, float e);

class world
{
public:
	world() : grav{}, world_width{}, world_height{} {}
	world(float world_width_meters, float world_height_meters, float gravity = -10);

	// make sure poly is not destroyed before world
	object *add_object(const abstract_shape &shape, glm::vec2 pos, glm::vec2 v_init, float angle, float w_init, float mass, glm::vec2 scale);

	// make sure poly is not destroyed before world
	// add object of infinite mass that stays in place
	object *add_static_object(const abstract_shape &shape, glm::vec2 pos, float angle, glm::vec2 scale);

	void add_constraint(std::unique_ptr<constraint> c) { constraints.push_back(std::move(c)); }

	void update(float dt)
	{
		if (dt > time_step)
			for (; dt > 0; dt -= time_step)
				update_internal();
	}

	float width() const { return world_width; }
	float height() const { return world_height; }
	float gravity() const { return grav; }

private:
	struct collision_pair
	{
		std::list<object>::iterator a, b;
		collision coll;
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