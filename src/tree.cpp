/*
 * tree.cpp
 *
 *  Created on: Apr 11, 2011
 *      Author: nedbingham
 */

#include "tree.h"

tree::tree()
{
	base = NULL;

	node_radius = 40;
	line_radius = 10;
	link_radius = 150;
	snap_speed = 5.0;
	cut_distance = 400.0;
	cut_velocity = 50.0;
	link_damping = 0.8;
	global_damping = 0.95;

	x = 0.0;
	y = 0.0;
}

tree::~tree()
{
	release(base);
}

void render(tree *t, node *n)
{
	n->vx += n->ax;
	n->vy += n->ay;

	if ((n->flags & 0x01) == 0)
	{
		n->x += n->vx;
		n->y += n->vy;
	}

	n->vx *= t->link_damping;
	n->vy *= t->link_damping;

	n->ax = 0;
	n->ay = 0;

	if (n->children && (n->flags & 0x02) == 0x02)
	{
		node *curr = n->children;
		if (curr != NULL)
			do
			{
				float l = sqrt((n->x - curr->x)*(n->x - curr->x) + (n->y - curr->y)*(n->y - curr->y));

				float a = (n->x - curr->x)/l;
				float b = (n->y - curr->y)/l;

				float f = (l - (float)t->link_radius)*t->snap_speed/(100.0);

				if ((n->flags & 0x01) == 0)
				{
					n->ax -= a*f;
					n->ay -= b*f;
				}

				if ((curr->flags & 0x01) == 0)
				{
					curr->ax += a*f;
					curr->ay += b*f;
				}

				line(t->line_radius*2, (int)(curr->x + t->x + 0.7*a*(float)t->node_radius), (int)(curr->y + t->y + 0.7*b*(float)t->node_radius), (int)(n->x + t->x), (int)(n->y + t->y), &t->button_icon);
				render(t, curr);
				curr = curr->next;
			} while (curr != n->children);
	}

	square(t->node_radius, (int)(n->x + t->x), (int)(n->y + t->y), &n->icon);
}

void render(tree *t)
{
	if ((t->flags & 0x01) == 0)
	{
		t->x += t->vx;
		t->y += t->vy;
	}

	t->vx *= t->global_damping;
	t->vy *= t->global_damping;

	float l, a, b, f;

	node *i0 = t->base->children, *i1, *i2;
	if (i0 != NULL)
		do
		{
			i1 = i0;
			while (i1 != NULL)
			{
				if (opened(i1))
				{
					i2 = i1->iter;
					while (i2 != NULL)
					{
						if (opened(i2))
						{
							l = sqrt((float)((i1->x - i2->x)*(i1->x - i2->x) + (i1->y - i2->y)*(i1->y - i2->y)));

							a = ((float)(i1->x - i2->x))/l;
							b = ((float)(i1->y - i2->y))/l;

							f = 5.0*t->snap_speed/l;

							if ((i1->flags & 0x01) == 0)
							{
								i1->ax += a*f;
								i1->ay += b*f;
							}

							if ((i2->flags & 0x01) == 0)
							{
								i2->ax -= a*f;
								i2->ay -= b*f;
							}
						}
						i2 = i2->iter;
					}
				}
				i1 = i1->iter;
			}
			i0 = i0->next;
		} while (i0 != t->base->children);

	i1 = t->base->children;
	if (i1 != NULL)
		do
		{
			render(t, i1);
			i1 = i1->next;
		} while (i1 != t->base->children);
}

node *node_select(tree *t, node *exclude, float x, float y)
{
	float l;
	float a = x - t->x;
	float b = y - t->y;

	node *curr;
	node *i0 = t->base->children;
	if (i0 != NULL)
		do
		{
			curr = i0;
			while (curr != NULL)
			{
				if (opened(curr))
				{
					l = (curr->x - a)*(curr->x - a) + (curr->y - b)*(curr->y - b);

					if (l < (float)t->node_radius*(float)t->node_radius && curr != exclude)
						return curr;
				}
				curr = curr->iter;
			}
			i0 = i0->next;
		} while (i0 != t->base->children);

	return NULL;
}

void node_cut(tree *t, float x1, float y1, float x2, float y2)
{
	node *curr;
	node *i0 = t->base->children;
	if (i0 != NULL)
		do
		{
			curr = i0;
			while (curr != NULL)
			{
				if (curr->parent && curr->parent->parent && opened(curr) && intersect(x1, y1, x2, y2, curr->x + t->x, curr->y + t->y, curr->parent->x + t->x, curr->parent->y + t->y))
				{
					liberate(curr, false);
					dominate(t->base, curr, false);
				}
				curr = curr->iter;
			}
			i0 = i0->next;
		} while (i0 != i0->parent->children);
}

void generate_lists(tree *t, node *iter, node *dter)
{
	if (iter == NULL)
	{
		node *n = t->base->children;
		do
		{
			generate_lists(t, n, NULL);
			n = n->next;
		} while (n != t->base->children);
	}
	else
	{
		iter->dter = dter;

		if (iter->children)
			iter->iter = iter->children;
		else if (iter->next != iter->parent->children)
			iter->iter = iter->next;
		else if (iter->parent && iter->parent->parent && iter->parent->parent != t->base && iter->parent->next != iter->parent->parent->children)
			iter->iter = iter->parent->next;

		if (iter->iter != NULL)
			generate_lists(t, iter->iter, iter);
	}
}

bool opened(node *n)
{
	if (n->parent)
	{
		if (n->parent->parent == NULL)
			return true;
		if ((n->parent->flags & 0x02) == 2)
			return opened(n->parent);
	}

	return false;
}

bool is_super(node *parent, node *child)
{
	if (!child->parent)
		return false;
	else if (parent == child->parent)
		return true;
	else
		return is_super(parent, child->parent);
}
