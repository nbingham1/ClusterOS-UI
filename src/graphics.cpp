/*
 * graphics.cpp
 *
 *  Created on: Apr 11, 2011
 *      Author: nedbingham
 */

#include "graphics.h"
#include "opengl_library.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned int _unit_square;
float unit_square_verts[8];
float unit_square_texts[8];
unsigned int _screen_width, _screen_height;

void initialize(int w, int h)
{
	glClearColor(0.0, 0.0, 0.0, 0.0);

	unit_square_verts[0*2 + 0] = -1.0;
	unit_square_verts[0*2 + 1] = -1.0;
	unit_square_texts[0*2 + 0] = 0.0;
	unit_square_texts[0*2 + 1] = 1.0;

	unit_square_verts[1*2 + 0] = 1.0;
	unit_square_verts[1*2 + 1] = -1.0;
	unit_square_texts[1*2 + 0] = 1.0;
	unit_square_texts[1*2 + 1] = 1.0;

	unit_square_verts[2*2 + 0] = 1.0;
	unit_square_verts[2*2 + 1] = 1.0;
	unit_square_texts[2*2 + 0] = 1.0;
	unit_square_texts[2*2 + 1] = 0.0;

	unit_square_verts[3*2 + 0] = -1.0;
	unit_square_verts[3*2 + 1] = 1.0;
	unit_square_texts[3*2 + 0] = 0.0;
	unit_square_texts[3*2 + 1] = 0.0;

	_unit_square = glGenLists(1);
	glNewList(_unit_square, GL_COMPILE);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, unit_square_verts);
		glTexCoordPointer(2, GL_FLOAT, 0, unit_square_texts);
		glDrawArrays(GL_POLYGON, 0, 4);
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glEndList();

	_screen_width = w;
	_screen_height = h;
}

void square(int radius, int x, int y, image *i)
{
	if (i)
	{
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, i->id);
	}
	glPushMatrix();
	glTranslatef((float)x, (float)y, 0.0);
	glScalef(radius, radius, 1.0);
	glCallList(_unit_square);
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
}

void line(int radius, int x1, int y1, int x2, int y2, image *i)
{
	if (i)
	{
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, i->id);
	}
	float length = floor(sqrt((float)((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1)))/2.0);
	float angle = atan2((float)y1 - (float)y2, (float)x2 - (float)x1)*180.0/3.1415926535898;
	glPushMatrix();
	glTranslatef((float)(x2 + x1)/2.0, (float)(y2 + y1)/2.0, 0.0);
	glRotatef(angle, 0.0, 0.0, -1.0);
	glScalef(length, (float)radius, 1.0);
	//glCallList(_unit_square);

	float l = sqrt((float)((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1)));
	float temp[8];
	for (int x = 0; x < 8; x++)
		temp[x] = x%2 == 0 ? unit_square_texts[x]*(l/(float)(radius*2)) : unit_square_texts[x];

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, unit_square_verts);
	glTexCoordPointer(2, GL_FLOAT, 0, temp);
	glDrawArrays(GL_POLYGON, 0, 4);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
}

unsigned char *rgb_tga(char *filename, int *w, int *h)
{
	FILE *file = fopen(filename, "rb");
	if (file == NULL)
	{
		printf("Could not open the file: %s\n", filename);
		exit(0);
	}

	unsigned char header[20];

	//read all 18 bytes of the header
	fread(header, sizeof(char), 18, file);

	//should be image type 2 (color) or type 10 (rle compressed color)
	if (header[2] != 2 && header[2] != 10)
	{
		fclose(file);
		exit(0);
	}

	if (header[0])
		fseek(file, header[0], SEEK_CUR);

	// get the size and bitdepth from the header
	int m_width = header[13] * 256 + header[12];
	int m_height = header[15] * 256 + header[14];
	int m_bpp = header[16] / 8;
	*w = m_width;
	*h = m_height;

	if (m_bpp != 3 && m_bpp != 4)
	{
		fclose(file);
		exit(0);
	}

	int imageSize = m_width * m_height * m_bpp;

	//allocate memory for image data
	unsigned char *data = new unsigned char[imageSize];

	//read the uncompressed image data if type 2
	if (header[2] == 2)
		fread(data, sizeof(char), imageSize, file);

	long ctpixel = 0,
		 ctloop = 0;

	//read the compressed image data if type 10
	if (header[2] == 10)
	{
		// stores the rle header and the temp color data
		unsigned char rle;
		unsigned char color[4];

		while (ctpixel < imageSize)
		{
			// reads the the RLE header
			fread(&rle, 1, 1, file);

			// if the rle header is below 128 it means that what folows is just raw data with rle+1 pixels
			if (rle < 128)
			{
				fread(&data[ctpixel], m_bpp, rle+1, file);
				ctpixel += m_bpp*(rle+1);
			}

			// if the rle header is equal or above 128 it means that we have a string of rle-127 pixels
			// that use the folowing pixels color
			else
			{
				// read what color we should use
				fread(&color[0], 1, m_bpp, file);

				// insert the color stored in tmp into the folowing rle-127 pixels
				ctloop = 0;
				while (ctloop < (rle-127))
				{
					data[ctpixel] = color[0];
					data[ctpixel+1] = color[1];
					data[ctpixel+2] = color[2];
					if (m_bpp == 4)
						data[ctpixel+3] = color[3];

					ctpixel += m_bpp;
					ctloop++;
				}
			}
		}
	}

	ctpixel=0;

	//Because TGA file store their colors in BGRA format we need to swap the red and blue color components
	unsigned char temp;
	while (ctpixel < imageSize)
	{
		temp = data[ctpixel];
		data[ctpixel] = data[ctpixel+2];
		data[ctpixel+2] = temp;
		ctpixel += m_bpp;
	}

	//close file
	fclose(file);

	return data;
}

void load(image *i, char *file)
{
	if (!i)
		return;

	int w, h;
	unsigned char *data = rgb_tga(file, &w, &h);

	strcpy(i->filename, file);

	glGenTextures(1, &i->id);
	glBindTexture(GL_TEXTURE_2D, i->id);
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, w, h, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	delete [] data;
}

bool intersect(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4)
{
	float d = (x2 - x1)*(y4 - y3) - (y2 - y1)*(x4 - x3);
	if (d == 0.0)
		return false;

	float r = ((y1 - y3)*(x4 - x3) - (x1 - x3)*(y4 - y3))/d;
	float s = ((y1 - y3)*(x2 - x1) - (x1 - x3)*(y2 - y1))/d;

	if (r < 0.0 || r > 1.0 || s < 0.0 || s > 1.0)
		return false;

	return true;
}
