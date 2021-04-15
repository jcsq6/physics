#pragma once
#include "object.h"

class world {
public:
	world(double width, double height, double px_per_meter);

	void update(double dt);

	void draw(SDL_Renderer *renderer) const;

	void add_object(const object &obj);
	void add_object(object&& obj);

	template<typename... Ts>
	void emplace_object(Ts&&... obj_args);

	a_vector<object>& get_objects();

	void lift(object* obj);
	void release(object* obj);

	void add_const_accel(const vec2& accel, std::string name);
	void remove_const_accel(std::string name);

	vec2 get_const_accel(std::string name);

private:
	std::map<std::string, vec2> const_accel;
	std::vector<bool> is_in_sim;
	a_vector<object> objects;
	mat<double> scaler;

	template<typename O>
	void forward_to_objects(O&& arg);
};

template<typename... Ts>
void world::emplace_object(Ts&&... obj_args) {
	objects.emplace_back(std::forward<Ts>(obj_args)...);
	for (const auto& accel : const_accel) {
		objects.back().add_const_accel(accel.second, accel.first);
	}
	is_in_sim.push_back(true);
}