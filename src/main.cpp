#include "world.h"
#include <iostream>

int main(int, char**) {
	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_Window* window = SDL_CreateWindow("template", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 960, 540, SDL_WINDOW_RESIZABLE);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

	const int px_count_horiz = 1920;
	const int px_count_vert = 1080;

	const int mtr_count_horiz = 96;
	const int mtr_count_vert = 54;

	SDL_RenderSetLogicalSize(renderer, px_count_horiz, px_count_vert);

	bool running = true;

	Uint64 now = SDL_GetPerformanceCounter();
	Uint64 last = 0;

	double dt = 0;

	//mat<double> meters_to_pixels = scale(px_count_horiz / mtr_count_horiz, px_count_vert / mtr_count_vert);

	//world mWorld(1920, 1080, px_count_horiz / mtr_count_horiz);

	//object pentagon(20, .1, poly::make_reg_poly({ mtr_count_horiz / 2, mtr_count_vert / 2 - 3 }, 2.5, 10, FULL));
	//object rect(20, .2, poly::make_rect({ 1, 1 }, 1, 1, FULL));

	//pentagon.addConstAccel({ 0, -10 }, "gravity");

	//pentagon.addImpulse({100000, 0}, .1);
	//rect.addImpulse({-100000, 0}, .1);

	//mWorld.addObj(pentagon);
	//mWorld.addObj(rect);

	poly hexagon = poly::make_reg_poly({ px_count_horiz / 2 - 300, px_count_vert / 2 - 300 }, 50, 6, COLLISION_REQS | DRAW_REQS);
	poly triangle = poly::make_reg_poly({ 0, 0 }, 30, 3, COLLISION_REQS | DRAW_REQS);
	poly rect = poly::make_rect({ px_count_horiz / 2 - 20, px_count_vert / 2 - 20 }, 40, 40, COLLISION_REQS | DRAW_REQS);
	poly circle = poly::make_reg_poly({ px_count_horiz / 2 - 500, 200 }, 50, 25, COLLISION_REQS | DRAW_REQS);

	while (running) {
		last = now;
		now = SDL_GetPerformanceCounter();
		dt = ((now - last) / (double)SDL_GetPerformanceFrequency());

		if (dt > .1) dt = .1;

		//mWorld.update(dt);

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				running = false;
				break;
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym) {
				case SDLK_ESCAPE:
					running = false;
					break;
				}
				break;
			case SDL_MOUSEMOTION:
				//triangle.set_rotation(vec2(event.motion.xrel, event.motion.yrel).angle());
				triangle.update_pos({(double)event.motion.x, (double)event.motion.y}, *triangle.begin());
				break;
			}
		}

		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_RenderClear(renderer);

		SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
		//mWorld.draw(renderer);
		if (auto t_h = poly::is_colliding(triangle, hexagon)) hexagon += t_h.min_translation_vec(1);
		if (auto t_r = poly::is_colliding(triangle, rect)) rect += t_r.min_translation_vec(1);
		if (auto t_c = poly::is_colliding(triangle, circle)) circle += t_c.min_translation_vec(1);

		if (auto r_h = poly::is_colliding(rect, hexagon)) {
			rect += r_h.min_translation_vec(0);
			hexagon += r_h.min_translation_vec(1);
		}
		if (auto r_c = poly::is_colliding(rect, circle)) {
			rect += r_c.min_translation_vec(0);
			circle += r_c.min_translation_vec(1);
		}
		if (auto h_c = poly::is_colliding(hexagon, circle)) {
			hexagon += h_c.min_translation_vec(0);
			circle += h_c.min_translation_vec(1);
		}

		SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
		poly::draw_poly(renderer, rect);

		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
		poly::draw_poly(renderer, hexagon);

		SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
		poly::draw_poly(renderer, circle);

		SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
		poly::draw_poly(renderer, triangle);

		SDL_RenderPresent(renderer);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
