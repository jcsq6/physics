#pragma once
#include "object.h"

class world {
public:
	world(double width, double height, double px_per_meter);

	void addObj(const object& obj);

	void update(double dt);

	void draw(SDL_Renderer *renderer);
private:
	a_vector<object> objects;
	mat<double> scaler;
};