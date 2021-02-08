#pragma once

//rgb structure
struct RGB
{
	RGB(unsigned char e, unsigned char d, unsigned char f):r(e), g(d), b(f)
	{
	
	}

	RGB():r(0), g(0), b(0)
	{
	
	}
	unsigned char r, g, b;
};



