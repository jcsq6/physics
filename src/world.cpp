#include "world.h"

world::world(double width, double height, double px_per_meter) : scaler{scale(px_per_meter, px_per_meter)} {
	objects.emplace_back(INFINITY, 1, poly::make_rect({ -100, 0 }, 100, height, COLLISION_REQS | NO_DRAW));
	objects.emplace_back(INFINITY, 1, poly::make_rect({ -100, -100 }, width + 100, 100, COLLISION_REQS | NO_DRAW));
	objects.emplace_back(INFINITY, 1, poly::make_rect({ width, -100 }, 100, height + 100, COLLISION_REQS | NO_DRAW));
	objects.emplace_back(INFINITY, 1, poly::make_rect({ -100, height }, width + 200, 100, COLLISION_REQS | NO_DRAW));
}
void world::addObj(const object& obj) {
	objects.push_back(obj);
}
void world::update(double dt) {
	double m1, m2, nVel, minr, impulse;
	collision n;
	vec2 imp, relVel;
	for (int obj1 = 0; obj1 < objects.size() - 1; obj1++) {
		for (int obj2 = obj1 + 1; obj2 < objects.size(); obj2++) {
			n = poly::is_colliding(poly::get_poly_path(objects[obj1].bounds, objects[obj1].bounds + objects[obj1].getVel() * dt, COLLISION_REQS), poly::get_poly_path(objects[obj2].bounds, objects[obj2].bounds + objects[obj2].getVel() * dt, COLLISION_REQS));
			if (n) {
				/*m1 = objects[obj1].getMassRecip();
				m2 = objects[obj2].getMassRecip();

				relVel = objects[obj2].getVel() - objects[obj1].getVel();

				nVel = dot(relVel, n.norm<0>());

				minr = std::min(objects[obj1].getRestitution(), objects[obj2].getRestitution());

				impulse = -(1 + minr) * nVel;
				impulse /= m1 + m2;

				imp = n.norm<0>() * impulse;

				objects[obj1].addVelocity(imp * -m1);
				objects[obj2].addVelocity(imp * m2);*/
			}
		}
	}
	for (auto& obj : objects) {
		if (obj.getMassRecip() != 0) obj.update(dt);
	}
}
void world::draw(SDL_Renderer* renderer) {
	for (const auto& obj : objects) {
		poly::draw_poly(renderer, obj.bounds * scaler);
	}
}