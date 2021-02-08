#pragma once
#include "Structure.hpp"
#define PI 3.1415926
#include <thread>

//structure of complex number
struct complex 
{
	unit_t real;
	unit_t img;

	complex(unit_t r, unit_t i) :real(r), img(i) 
	{
	
	}

};

//color templates
float color_a[3] = {0.5, 0.5, 0.5};
float color_b[3] = {0.5, 0.5, 0.5};
float color_c[3] = {1.0, 1.0, 1.0};
float color_d[3] = {0.0, 0.1, 0.2};

//convert grid to rgb color
inline RGB gridToColor(float grid_val) 
{
	float r, g, b;
	r = color_a[0] + (color_b[0] * cosf(2 * PI * ((color_c[0] * grid_val) + color_d[0])));
	g = color_a[1] + (color_b[1] * cosf(2 * PI * ((color_c[1] * grid_val) + color_d[1])));
	b = color_a[2] + (color_b[2] * cosf(2 * PI * ((color_c[2] * grid_val) + color_d[2])));

	RGB ret(r * 255.0, g * 255.0, b * 255.0);
	return ret;
}

//add two complex numbers
inline complex imgAdd(const complex& a, const complex& b) 
{
	return complex(a.real + b.real, a.img + b.img);
}

//multiply two complex numbers
inline complex imgMultiply(const complex& a, const complex& b) 
{
	return complex((a.real * b.real) - (a.img * b.img), (a.real * b.img) + (a.img * b.real));
}

//square complex number
inline complex imgSquare(const complex& a) 
{
	return imgMultiply(a, a);
}


void renderThread(unit_t delta_x, unit_t delta_y, int from_y, int to_y, int w, unit_t left, unit_t top, RGB* frame) 
{
	unit_t x_pos = 0;
	unit_t y_pos = delta_y * from_y;

	for (int y = from_y; y < to_y; y++) 
	{
		x_pos = 0;
		for (int x = 0; x < w; x++) 
		{
			complex c(left + x_pos, top + y_pos);
			complex z(0, 0);

			int max_iter = 0;
			for (max_iter = 0; max_iter < iteration; max_iter++)
			{
				z = imgAdd(imgSquare(z), c);

				//check if number is out of bound
				if (((z.real * z.real) + (z.img * z.img)) > BOUND_SQUARE)
				{
					break;
				}

			}

			frame[(y * w) + x] = (max_iter == iteration) ? RGB(0, 0, 0) : gridToColor((float)max_iter / iteration);
			x_pos += delta_x;		
		}	
		y_pos += delta_y;
	}

}



void renderImage(unit_t topLeft[2], unit_t rightBottom[2],SDL_Surface* buffer)
{
	RGB* frame = (RGB*)buffer->pixels;
	int w = buffer->w;
	int h = buffer->h;

	//distance from right to left 
	//and the distance from top to bottom
	unit_t distance_x = rightBottom[0] - topLeft[0];
	unit_t distance_y = rightBottom[1] - topLeft[1];

	//x interval and y interval
	unit_t delta_x = distance_x / w;
	unit_t delta_y = distance_y / h;

	std::thread threads[THREADS];

	float task_per_thread = (float)h / THREADS;
	float current = 0;

	//assign tasks
	for (int i = 0; i < THREADS; i++) 
	{
		threads[i] = std::thread(renderThread, delta_x, delta_y, current, current + task_per_thread, w, topLeft[0], topLeft[1], frame);
		current += task_per_thread;
	}
	
	//wait all threads
	for (int i = 0; i < THREADS; i++) 
	{
		threads[i].join();
	}
	
}

