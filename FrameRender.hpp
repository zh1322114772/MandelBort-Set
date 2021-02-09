#pragma once
#include "Structure.hpp"
#define PI 3.1415926
#include <thread>


#ifndef USE_AVX
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
#endif


//use avx512
#ifdef USE_AVX
#include <immintrin.h>

//structure of complex number
struct complex
{
	__m256d real;
	__m256d img;

	complex(__m256d& r, __m256d& i) :real(r), img(i)
	{

	}

	complex(__m256d&& r, __m256d&& i) :real(r), img(i)
	{

	}

	complex() 
	{
		real = { 0.0 };
		img = { 0.0 };
	}

};

//avx512 stuff
__m256d increase_one = _mm256_set1_pd(1);
__m256d bound_square = _mm256_set1_pd(BOUND_SQUARE);

//color templates
//use xmm 
__m256 color_a = { 0.5, 0.5, 0.5, 0.0, 0.5, 0.5, 0.5, 0.0 };
__m256 color_b = { 0.5, 0.5, 0.5, 0.0, 0.5, 0.5, 0.5, 0.0 };
__m256 color_c = { 1.0, 1.0, 1.0, 0.0, 1.0, 1.0, 1.0, 0.0 };
__m256 color_d = { 0.0, 0.1, 0.2, 0.0, 0.0, 0.1, 0.2, 0.0 };
__m256 oneTo255 = { 255.0, 255.0, 255.0, 0.0, 255.0, 255.0, 255.0, 0.0 };
__m256 twoPi = { 2 * PI, 2 * PI, 2 * PI, 0.0, 2 * PI, 2 * PI, 2 * PI, 0.0 };

//convert grid to rgb color
inline void gridToColor(__m256d& grid, RGB* result)
{
	__m256 firstTwo = {grid.m256d_f64[0], grid.m256d_f64[0], grid.m256d_f64[0], 0.0, grid.m256d_f64[1], grid.m256d_f64[1], grid.m256d_f64[1], 0.0};
	__m256 lastTwo = { grid.m256d_f64[2], grid.m256d_f64[2], grid.m256d_f64[2], 0.0, grid.m256d_f64[3], grid.m256d_f64[3], grid.m256d_f64[3], 0.0};

	__m256i res = _mm256_cvtps_epi32(_mm256_mul_ps(_mm256_add_ps(color_a, _mm256_mul_ps(color_b, _mm256_cos_ps(_mm256_mul_ps(twoPi, _mm256_add_ps(_mm256_mul_ps(color_c, firstTwo), color_d))))), oneTo255));
	__m256i res1 = _mm256_cvtps_epi32(_mm256_mul_ps(_mm256_add_ps(color_a, _mm256_mul_ps(color_b, _mm256_cos_ps(_mm256_mul_ps(twoPi, _mm256_add_ps(_mm256_mul_ps(color_c, lastTwo), color_d))))), oneTo255));


	*result = (grid.m256d_f64[0] == 1)? RGB(0,0,0) : RGB(res.m256i_i32[0], res.m256i_i32[1], res.m256i_i32[2]);
	*(result + 1) = (grid.m256d_f64[1] == 1) ? RGB(0, 0, 0) : RGB(res.m256i_i32[4], res.m256i_i32[5], res.m256i_i32[6]);
	*(result + 2) = (grid.m256d_f64[2] == 1) ? RGB(0, 0, 0) : RGB(res1.m256i_i32[0], res1.m256i_i32[1], res1.m256i_i32[2]);
	*(result + 3) = (grid.m256d_f64[3] == 1) ? RGB(0, 0, 0) : RGB(res1.m256i_i32[4], res1.m256i_i32[5], res1.m256i_i32[6]);
}

//add two complex numbers
inline complex imgAdd(const complex& a, const complex& b)
{
	return complex(_mm256_add_pd(a.real, b.real), _mm256_add_pd(a.img, b.img));
}

//multiply two complex numbers
inline complex imgMultiply(const complex& a, const complex& b)
{
	return complex(_mm256_sub_pd(_mm256_mul_pd(a.real, b.real), _mm256_mul_pd(a.img, b.img)), _mm256_add_pd(_mm256_mul_pd(a.real, b.img), _mm256_mul_pd(a.img, b.real)));
	//return complex((a.real * b.real) - (a.img * b.img), (a.real * b.img) + (a.img * b.real));
}

//square complex number
inline complex imgSquare(const complex& a)
{
	return imgMultiply(a, a);
}

inline void doAvx(complex& c, complex& z, __m256d& result, __m256d& max_iteration) 
{
	union doubleToUnsignedLong 
	{
		__m256d d;
		__m256i l;
	};

	int max_iter = 0;
	doubleToUnsignedLong in_bound;
	__m256d current_iteration = _mm256_set1_pd(0);
	//flag _CMP_LT_OS, less than
	const static int imm8_flag = 2;
	
	for (max_iter = 0; max_iter < iteration; max_iter++)
	{
		__m256d add_one = {0};
		z = imgAdd(imgSquare(z), c);
		in_bound.d = _mm256_cmp_pd(_mm256_add_pd(_mm256_mul_pd(z.real, z.real), _mm256_mul_pd(z.img, z.img)), bound_square, imm8_flag);
		
		int to_break = 4;

		for (int i = 0; i < 4; i++) 
		{
			if (in_bound.l.m256i_u64[i] > 0) 
			{
				current_iteration.m256d_f64[i] += 1;
			}
			else 
			{
				to_break--;
			}
		
		}

		if (!to_break) break;

	}

	result = _mm256_div_pd(current_iteration, max_iteration);
}

void renderThread(double delta_x, double delta_y, int from_y, int to_y, int w, double left, double top, RGB* frame)
{
	double x_pos = 0;
	double y_pos = delta_y * from_y;
	int align_offset = w % 8;

	//set x_offset and iteration vectors for avx512
	__m256d x_offsets = { 0.0, delta_x, 2 * delta_x, 3 * delta_x};
	__m256d iterations = _mm256_set1_pd(iteration);

	for (int y = from_y; y < to_y; y++)
	{

		complex c(_mm256_add_pd(_mm256_set1_pd(left), x_offsets), _mm256_set1_pd(top + y_pos));
		complex z = complex();
		__m256d result;
		
		for (int x = align_offset; x < w; x+=4)
		{
			x_pos = x * delta_x;
			c.real = _mm256_add_pd(_mm256_set1_pd(left + x_pos), x_offsets);
			
			z = complex();
			doAvx(c, z, result, iterations);
			
			gridToColor(result, (frame + (y * w) + x));

			
		}
		y_pos += delta_y;
	}

}



void renderImage(unit_t topLeft[2], unit_t rightBottom[2], SDL_Surface* buffer)
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
#endif 
