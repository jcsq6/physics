#include "world.h"
#include <iostream>

int main(int, char**) {
	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_Window* window = SDL_CreateWindow("template", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 960, 540, SDL_WINDOW_RESIZABLE);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

	SDL_ShowCursor(SDL_DISABLE);

	const int px_count_horiz = 1920;
	const int px_count_vert = 1080;

	const int mtr_count_horiz = 96;
	const int mtr_count_vert = 54;

	SDL_RenderSetLogicalSize(renderer, px_count_horiz, px_count_vert);

	bool running = true;

	Uint64 now = SDL_GetPerformanceCounter();
	Uint64 last = 0;

	double dt = 0;

	mat meters_to_pixels = scale(px_count_horiz / mtr_count_horiz, px_count_vert / mtr_count_vert);

	world m_world(1920, 1080, 1);
	m_world.add_const_accel({ 0, 400 }, "gravity");

	m_world.emplace_object(100, .85, poly::make_reg_poly({ px_count_horiz / 2 - 50, px_count_vert / 2 - 50 }, 50, 5, M_PI, COLLISION_REQS | DRAW_REQS), vec2(0, 0), 255, 0, 0);
	m_world.emplace_object(100, .85, poly::make_reg_poly({ px_count_horiz / 2 - 50, px_count_vert / 2 - 200 }, 50, 5, M_PI, COLLISION_REQS | DRAW_REQS), vec2(0, 0), 255, 255, 0);
	
	m_world.emplace_object(75, .85, poly::make_reg_poly({ px_count_horiz / 2 - 200, px_count_vert / 2 - 50 }, 30, 20, M_PI, COLLISION_REQS | DRAW_REQS), vec2(40, 0), 255, 0, 255);
	m_world.emplace_object(200, .85, poly::make_reg_poly({ px_count_horiz / 2 - 200, px_count_vert / 2 - 400 }, 100, 20, M_PI, COLLISION_REQS | DRAW_REQS), vec2(100, 100), 0, 255, 0);

	m_world.emplace_object(100, .85, poly::make_rect({ 50, 50 }, 50, 70, COLLISION_REQS | DRAW_REQS), vec2(100, 20), 155, 34, 255);
	m_world.emplace_object(100, .85, poly::make_rect({ px_count_horiz - 100, px_count_vert - 100 }, 70, 50, COLLISION_REQS | DRAW_REQS), vec2(40, 40), 0, 255, 255);

	object mouse(0, 0, poly(COLLISION_REQS | DRAW_REQS, vec2(0, -20), vec2(-10, -10), vec2(10, -10), vec2(-5, 15), vec2(5, 15)), { 0, 0 });
	mouse.bounds.rotate(-M_PI_4, mouse.bounds.center());

	object* grabbed_obj = nullptr;

	while (running) {
		last = now;
		now = SDL_GetPerformanceCounter();
		dt = ((now - last) / (double)SDL_GetPerformanceFrequency());

		if (dt > .03) dt = .03;

		m_world.update(dt);

		if (grabbed_obj) {
			grabbed_obj->bounds.update_pos(mouse.bounds[0], grabbed_obj->bounds.center());
			grabbed_obj->vel = mouse.vel * 25;
		}

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
				}
				break;
			case SDL_MOUSEMOTION:
				mouse.bounds.update_pos({(double)event.motion.x, (double)event.motion.y}, mouse.bounds.center());
				mouse.vel.x = event.motion.xrel;
				mouse.vel.y = event.motion.yrel;
				break;
			case SDL_MOUSEBUTTONDOWN:
				for (auto& obj : m_world.get_objects()) {
					if (poly::is_colliding(mouse.bounds, obj.bounds)) {
						grabbed_obj = &obj;
						m_world.lift(grabbed_obj);
						break;
					}
				}
				break;
			case SDL_MOUSEBUTTONUP:
				if (grabbed_obj)
					m_world.release(grabbed_obj);
				grabbed_obj = nullptr;
				break;
			}
		}

		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_RenderClear(renderer);

		m_world.draw(renderer);

		SDL_SetRenderDrawColor(renderer, mouse.r, mouse.g, mouse.b, mouse.a);
		poly::draw_poly(renderer, mouse.bounds);

		SDL_RenderPresent(renderer);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
