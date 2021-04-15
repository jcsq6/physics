#include "world.h"

world::world(double width, double height, double px_per_meter = 1) : scaler{ scale(px_per_meter, px_per_meter) }, is_in_sim(4, true) {
	objects.emplace_back(INFINITY, 1, poly::make_rect({ -100, 0 }, 100, height, COLLISION_REQS | NO_DRAW));
	objects.emplace_back(INFINITY, 1, poly::make_rect({ -100, -100 }, width + 100, 100, COLLISION_REQS | NO_DRAW));
	objects.emplace_back(INFINITY, 1, poly::make_rect({ width, -100 }, 100, height + 100, COLLISION_REQS | NO_DRAW));
	objects.emplace_back(INFINITY, 1, poly::make_rect({ -100, height }, width + 200, 100, COLLISION_REQS | NO_DRAW));
}
void world::update(double dt) {
	collision n;
	double vj;
	vec2 rel_vel;
	poly o1_path;
	poly o2_path;
	for (int obj1 = 0; obj1 < objects.size() - 1; obj1++) {
		for (int obj2 = obj1 + 1; obj2 < objects.size(); obj2++) {
			if ((objects[obj1].m_recip == 0 && objects[obj2].m_recip == 0) || !is_in_sim[obj1] || !is_in_sim[obj2]) continue;

			o1_path = poly::get_poly_path(objects[obj1].bounds, objects[obj1].bounds + objects[obj1].vel * dt, COLLISION_REQS);
			o2_path = poly::get_poly_path(objects[obj2].bounds, objects[obj2].bounds + objects[obj2].vel * dt, COLLISION_REQS);

			n = poly::is_colliding(o1_path, o2_path);
			
			if (n) {
				rel_vel = objects[obj1].vel - objects[obj2].vel;
				vj = dot(rel_vel * (-1 - (objects[obj1].restitution * objects[obj2].restitution)), n.normal()) / (objects[obj1].m_recip + objects[obj2].m_recip);

				objects[obj1].vel += n.normal() * ( objects[obj1].m_recip * vj);
				objects[obj2].vel += n.normal() * (-objects[obj2].m_recip * vj);
			}
		}
	}
	for (int obj = 0; obj < objects.size(); obj++) {
		if (objects[obj].m_recip && is_in_sim[obj]) objects[obj].update(dt);
	}
}
void world::draw(SDL_Renderer* renderer) const {
	for (const auto& obj : objects) {
		SDL_SetRenderDrawColor(renderer, obj.r, obj.g, obj.b, obj.a);
		poly::draw_poly(renderer, obj.bounds * scaler);
	}
}

void world::add_object(const object &obj) {
	forward_to_objects(std::move(obj));
}

void world::add_object(object&& obj) {
	forward_to_objects(std::move(obj));
}

a_vector<object>& world::get_objects() {
	return objects;
}

void world::lift(object* obj) {
	for (int i = 0; i < objects.size(); i++) {
		if (&objects[i] == obj) {
			is_in_sim[i] = false;
			return;
		}
	}
}

void world::release(object* obj) {
	for (int i = 0; i < objects.size(); i++) {
		if (&objects[i] == obj) {
			is_in_sim[i] = true;
			return;
		}
	}
}

void world::add_const_accel(const vec2& accel, std::string name){
	const_accel[name] = accel;
	for (auto& obj : objects) {
		obj.add_const_accel(accel, name);
	}
}

void world::remove_const_accel(std::string name){
	for (auto& obj : objects) {
		obj.remove_const_accel(name);
	}
	const_accel.erase(name);
}

vec2 world::get_const_accel(std::string name) {
	return const_accel.at(name);
}

template<typename O>
void world::forward_to_objects(O&& arg){
	object tmp{ std::forward<O>(arg) };
	for (const auto& accel : const_accel) {
		tmp.add_const_accel(accel.second, accel.first);
	}
	objects.push_back(std::move(tmp));
	is_in_sim.push_back(true);
}