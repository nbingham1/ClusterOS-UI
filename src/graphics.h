/*
 * graphics.h
 *
 *  Created on: Apr 11, 2011
 *      Author: nedbingham
 */

#ifndef graphics_h
#define graphics_h

struct image
{
	char filename[256];
	unsigned int id;
};

void initialize(int w, int h);
void square(int radius, int x, int y, image *i);
void line(int radius, int x1, int y1, int x2, int y2, image *i);

bool intersect(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4);

void load(image *i, char *file);

#endif
