/*
 * tree.h
 *
 *  Created on: Apr 11, 2011
 *      Author: nedbingham
 */

#include "graphics.h"
#include "standard.h"
#include "node.h"

#ifndef tree_h
#define tree_h

struct tree
{
	tree();
	~tree();

	node *base;
	image link_icon;
	image button_icon;

	int node_radius;
	int line_radius;
	int link_radius;

	float link_damping;
	float global_damping;

	float cut_distance;
	float cut_velocity;
	float snap_speed;

	char flags; // [r][r][r][r][r][r][r][selected]

	float x;
	float y;
	float vx;
	float vy;
};

void render(tree *t, node *n);
void render(tree *t);
node *node_select(tree *t, node *exclude, float x, float y);
void node_cut(tree *t, float x1, float y1, float x2, float y2);
void generate_lists(tree *t, node *iter, node *dter);
bool opened(node *n);
bool is_super(node *parent, node *child);

#endif
