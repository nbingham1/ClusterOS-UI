/*
 * node.h
 *
 *  Created on: Apr 11, 2011
 *      Author: nedbingham
 */

#include "graphics.h"
#include "standard.h"

#ifndef node_h
#define node_h

struct node
{
	node();
	~node();

	char name[32];
	image icon;

	char flags;	// [r][r][r][r][r][r][open][selected]

	float x, y;
	float vx, vy;
	float ax, ay;

	node *children;
	node *parent;

	node *next;
	node *prev;

	node *iter;
	node *dter;
};

void dominate(node *parent, node *child, bool i);
void liberate(node *child, bool i);

void load(node *parent);
void unload(node *parent);

void release(node *n);

#endif
