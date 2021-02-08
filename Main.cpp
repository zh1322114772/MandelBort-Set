#include <SDL.h>
#include <iostream>
#include "Structure.hpp"
#include <string>

#define USE_DOUBLE
#define BOUND_SQUARE 4.0
#define THREADS 16
int iteration = 64;

//use either single precision or double precision
#ifdef USE_DOUBLE
using unit_t = double;
#else
using unit_t = float;
#endif

#include "FrameRender.hpp"
//frame render function
void renderImage(unit_t topLeft[2], unit_t rightBottom[2], SDL_Surface* frame);

//window properties
SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
unit_t renderArea[2] = {2, 2};
unit_t renderOffset[2] = {0, 0};
int windowSize[2] = { 1000, 1000};
SDL_Surface* frameBuffer = SDL_CreateRGBSurface(0, windowSize[0], windowSize[1], 24, 0xff0000, 0x00ff00, 0x00ff, 0);

//sdl stuff
SDL_Event e;
SDL_Surface* screenSurface;

int main(int argc, char* argv[])
{
	
	//init sdl
	if (SDL_Init(SDL_INIT_VIDEO) < 0) 
	{
		printf("SDL_Error: %s\n", SDL_GetError());
		return -1;
	}

	//create window
	window = SDL_CreateWindow("Mandelbrot Set", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, windowSize[0], windowSize[1], SDL_WINDOW_SHOWN);
	if (window == NULL)
	{
		printf("SDL_Error: %s\n", SDL_GetError());
		return -1;
	}

	//create renderer
	renderer = SDL_CreateRenderer(window, -1, 0);
	if (renderer == NULL) 
	{
		printf("SDL_Error: %s\n", SDL_GetError());
		return -1;
	}


	screenSurface = SDL_GetWindowSurface(window);
	SDL_Surface* test = SDL_LoadBMP("asd.bmp");
	bool dont_quit = true;

	//mouse motion
	int mouse_x = 0;
	int mouse_y = 0;

	//key events
	bool zoom = false;
	bool shrink = false;
	unit_t zoom_factor = 0.5;
	unit_t move_speed = 1.0;
	unit_t scale = 1.0;
	bool up = false;
	bool down = false;
	bool left = false;
	bool right = false;

	//render time interval
	int last_tick = 0;
	int current_tick = 0;
	double delta_t = 0;

	//render loop
	while (dont_quit)
	{

		//calculate the time interval
		current_tick = SDL_GetTicks();
		delta_t = (double)(current_tick - last_tick) / 1000;
		last_tick = current_tick;

		//event process
		while (SDL_PollEvent(&e)) 
		{
			switch (e.type) 
			{
			case SDL_QUIT:dont_quit = false; break;
			case SDL_MOUSEMOTION:
				mouse_x = e.motion.x;
				mouse_y = e.motion.y;
				break;
			case SDL_KEYDOWN:
				switch (e.key.keysym.sym)
				{
				case SDLK_z: zoom = true; break;
				case SDLK_x: shrink = true; break;
				case SDLK_w: up = true; break;
				case SDLK_a: left = true; break;
				case SDLK_s: down = true; break;
				case SDLK_d: right = true; break;
				}
				break;
			case SDL_KEYUP:
				switch (e.key.keysym.sym)
				{
				case SDLK_z: zoom = false; break;
				case SDLK_x: shrink = false; break;
				case SDLK_w: up = false; break;
				case SDLK_a: left = false; break;
				case SDLK_s: down = false; break;
				case SDLK_d: right = false; break;
				}
				break;
			}

		}

		//ajust iteration
		iteration = fmaxf((128.0 * log10f(1.0f / scale)), 32.0);


		if (zoom || shrink) 
		{
			unit_t s = ((zoom) ? -zoom_factor : zoom_factor) * delta_t;
			renderArea[0] += renderArea[0] * s;
			renderArea[1] += renderArea[1] * s;
			scale += scale * s;
		}
		
		//render window offset
		renderOffset[0] += move_speed * renderArea[0] * (-left + right) * delta_t;
		renderOffset[1] += move_speed * renderArea[0] * (-up + down) * delta_t;

		//set render range
		unit_t range0[2] = { renderOffset[0] - renderArea[0], renderOffset[1] - renderArea[1] };
		unit_t range1[2] = { renderOffset[0] + renderArea[0], renderOffset[1] + renderArea[1] };
		
		renderImage(range0, range1, frameBuffer);

		SDL_BlitSurface(frameBuffer, &(frameBuffer->clip_rect), screenSurface, NULL);
		SDL_UpdateWindowSurface(window);

		//update title
		SDL_SetWindowTitle(window, ("Mandelbrot Set -FPS: " + std::to_string(1.0/delta_t) + " -Zoom: " + std::to_string(1.0 / scale) + "x -Iteration: " + std::to_string(iteration)).c_str());

	}

	//Destroy window
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;

}