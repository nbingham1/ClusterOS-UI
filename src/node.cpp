/*
 * node.cpp
 *
 *  Created on: Apr 11, 2011
 *      Author: nedbingham
 */

#include "node.h"

node::node()
{
	children = NULL;
	parent = NULL;
	next = this;
	prev = this;
	iter = NULL;
	dter = NULL;

	flags = 0;

	x = 0.0;
	y = 0.0;
	vx = 0.0;
	vy = 0.0;
	ax = 0.0;
	ay = 0.0;

	name[0] = '\0';
}
node::~node()
{
	unload(this);
}

void dominate(node *parent, node *child, bool i)
{
	node *last = child;
	if (!i)
	{
		while (last->children != NULL)
			last = last->children->prev;
	}

	if (parent->children)
	{
		child->parent = parent;
		child->next = parent->children;
		child->prev = parent->children->prev;

		parent->children->prev->next = child;
		parent->children->prev = child;
		parent->children = child;
	}
	else
	{
		child->parent = parent;
		child->next = child;
		child->prev = child;

		parent->children = child;

		parent->flags |= 0x02;
	}

	if (!i)
	{
		if (parent->iter)
			parent->iter->dter = last;
		last->iter = parent->iter;
		if (parent->parent != NULL)
		{
			parent->iter = child;
			child->dter = parent;
		}
		else
			child->dter = NULL;
	}
}

void liberate(node *child, bool i)
{
	if (!i)
	{
		node *last = child;
		while (last->children != NULL)
			last = last->children->prev;

		if (child->dter)
			child->dter->iter = last->iter;

		if (last->iter)
			last->iter->dter = child->dter;
		last->iter = NULL;
		child->dter = NULL;
	}

	if (child->next == child)
	{
		child->parent->children = NULL;
		child->parent = NULL;
	}
	else
	{
		if (child->parent->children == child)
			child->parent->children = child->next;

		child->prev->next = child->next;
		child->next->prev = child->prev;

		child->next = child;
		child->prev = child;

		child->parent = NULL;
	}
}

void load(node *parent)
{

}

void unload(node *parent)
{
	node *curr = parent->children, *prev;
	parent->children = NULL;
	if (curr)
	{
		curr->prev->next = NULL;
		curr->prev = NULL;

		while (curr != NULL)
		{
			prev = curr;
			curr = curr->next;
			release(prev);
			prev = NULL;
		}
	}
}

void release(node *n)
{
	unload(n);
	free(n);
}
