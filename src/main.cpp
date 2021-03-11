#include "mat.h"
#include "world.h"
#include <iostream>

int main(int, char**) {
	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_Window* window = SDL_CreateWindow("template", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 960, 540, SDL_WINDOW_RESIZABLE);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

	const unsigned int px_count_horiz = 1920;
	const unsigned int px_count_vert = 1080;

	SDL_RenderSetLogicalSize(renderer, px_count_horiz, px_count_vert);

	bool running = true;

	Uint64 now = SDL_GetPerformanceCounter();
	Uint64 last = 0;

	double dt = 0;

	mat<5> mat5;

	for (int y = 0; y < 5; y++) {
		for (int x = 0; x < 5; x++) {
			std::cout << mat5[x][y] << ", ";
		}
		std::cout << std::endl;
	}
	

	//world mWorld(cartesian_to_graphic, meters_to_pixels);

	//object pentagon(20, .1, poly::make_reg_poly({ meter_count_horiz / 2, 27 - 3 }, 2.5, 10, FULL));
	//object rect(20, .2, poly::make_rect({ 1, 1 }, 1, 1, FULL));

	//pentagon.addConstAccel({ 0, -10 }, "gravity");

	//pentagon.addImpulse({100000, 0}, .1);
	//rect.addImpulse({-100000, 0}, .1);

	//mWorld.addObj(pentagon);
	//mWorld.addObj(rect);

	poly hexagon = poly::make_reg_poly({ 24, 1 }, 1, 6, COLLISION_REQS | DRAW_REQS);
	poly rect = poly::make_rect({ 24 - 2, 13.5 - 2 }, 4, 4, COLLISION_REQS | DRAW_REQS);

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
			}
		}

		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_RenderClear(renderer);

		SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
		//mWorld.draw(renderer);

		SDL_RenderPresent(renderer);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
