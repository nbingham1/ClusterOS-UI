/*
 * main.cpp
 *
 *  Created on: Apr 11, 2011
 *      Author: nedbingham
 */

#include "graphics.h"
#include "opengl_library.h"
#include "standard.h"
#include "tree.h"

tree ui;
node *selected_node = NULL;
float mx = 0.0, my = 0.0;
bool leftb = false, rightb = false, middleb = false;

int current_frame = 0;

int start_frame = 0;

int tap1_frame = 0, tap2_frame = 0;

void init()
{
	srand(0);
	initialize(1366, 768);
	glMatrixMode(GL_PROJECTION_MATRIX);
	glLoadIdentity();
	glOrtho(0.0, (float)1366, (float)768, 0.0, 0.0, 1.0);
	glMatrixMode(GL_MODELVIEW_MATRIX);

	glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GEQUAL, 0.05);
	glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	FILE *file = fopen("file", "rb");
	if (!file)
		exit(0);

	char num = fgetc(file);
	node *n = new node[num];
	char imagefile[256];
	char id, id1;
	printf("%d\n", num);
	int j;
	for (char l = 0; l < num; l++)
	{
		id = fgetc(file) - 1;
		j = 0;
		while ((n[id].name[j++] = fgetc(file)) != '\0');
		n[id].name[j++] = '\0';
		j = 0;
		printf("%d %s\n", id, n[id].name);
		while ((imagefile[j++] = fgetc(file)) != '\0');
		imagefile[j++] = '\0';
		printf("%d %s\n", id, imagefile);
		load(&n[id].icon, imagefile);
		id1 = fgetc(file) - 1;
		printf("%d %d\n", id, id1);
		n[id].children = 0x0;
		n[id].next = &n[id];
		n[id].prev = &n[id];
		n[id].parent = 0x0;
		n[id].x = 1366.0/2.0 + (float)(rand()%201 - 100)/100.0;
		n[id].y = 768.0/2.0 + (float)(rand()%201 - 100)/100.0;
		n[id].vx = 0;
		n[id].vy = 0;
		n[id].ax = 0;
		n[id].ay = 0;
		n[l].iter = NULL;
		if (id1 >= 0)
			dominate(&n[id1], &n[id], true);
	}

	fclose(file);

	ui.base = n;
	load(&ui.link_icon, "connect.tga");
	load(&ui.button_icon, "button.tga");
	generate_lists(&ui, NULL, NULL);
}

void displayfunc()
{
	current_frame++;
	glClear(GL_COLOR_BUFFER_BIT);
	render(&ui);
	glutSwapBuffers();
}

void reshape(int w, int h)
{
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION_MATRIX);
	glLoadIdentity();
	glOrtho(0.0, (float)w, (float)h, 0.0, 0.0, 1.0);
	glMatrixMode(GL_MODELVIEW_MATRIX);
}

void pmotionfunc(int x, int y)
{
}

void mousefunc(int button, int state, int x, int y)
{
	node *n;

	switch (state)
	{
	case GLUT_DOWN:
		switch (button)
		{
		case GLUT_LEFT_BUTTON:
			selected_node = node_select(&ui, NULL, (float)x, (float)y);
			if (selected_node)
				selected_node->flags |= 0x01;
			mx = (float)x;
			my = (float)y;

			start_frame = current_frame;

			tap1_frame = tap2_frame;
			tap2_frame = current_frame;

			/*if (tap2_frame - tap1_frame < 800)
			{
				node *n = new node();

				strcpy(n->name, "Untitled Node");
				load(&n->icon, "Sol.tga");
				n->children = NULL;
				n->next = n;
				n->prev = n;
				n->parent = NULL;
				n->x = x - ui.x;
				n->y = y - ui.y;
				n->vx = 0;
				n->vy = 0;
				n->ax = 0;
				n->ay = 0;
				n->iter = NULL;
				n->dter = NULL;
				dominate(ui.base, n, true);
			}*/

			leftb = true;
			break;
		case GLUT_RIGHT_BUTTON:
			ui.flags |= 0x01;
			mx = (float)x;
			my = (float)y;
			rightb = true;
			break;
		case GLUT_MIDDLE_BUTTON:
			middleb = true;
			break;
		}
		break;
	case GLUT_UP:
		switch (button)
		{
		case GLUT_LEFT_BUTTON:
			if (selected_node)
			{
				if (selected_node->parent == ui.base)
				{
					n = node_select(&ui, selected_node, (float)x, (float)y);
					if (n && !is_super(selected_node, n))
					{
						liberate(selected_node, false);
						dominate(n, selected_node, false);
					}
				}

				if (current_frame - start_frame < 400)
					selected_node->flags ^= 0x02;
				start_frame = 0;

				selected_node->flags &= 0xfe;
				selected_node = NULL;
			}

			leftb = false;
			break;
		case GLUT_RIGHT_BUTTON:
			ui.flags &= 0xfe;
			rightb = false;
			break;
		case GLUT_MIDDLE_BUTTON:
			middleb = false;
			break;
		}
		break;
	}
}

void motionfunc(int x, int y)
{
	float dx = (float)x - mx;
	float dy = (float)y - my;
	float l;

	if (leftb)
	{
		if (selected_node)
		{
			selected_node->x += dx;
			selected_node->y += dy;

			selected_node->vx = dx*0.5;
			selected_node->vy = dy*0.5;

			l = (selected_node->x - selected_node->parent->x)*(selected_node->x - selected_node->parent->x) + (selected_node->y - selected_node->parent->y)*(selected_node->y - selected_node->parent->y);
			if (selected_node->parent && selected_node->parent->parent && (l > ui.cut_distance*ui.cut_distance || (dx*dx/4.0 + dy*dy/4.0) > ui.cut_velocity*ui.cut_velocity))
			{
				liberate(selected_node, false);
				dominate(ui.base, selected_node, false);
			}
		}
		else
			node_cut(&ui, (float)x, (float)y, mx, my);
	}
	if (rightb)
	{
		ui.x += dx;
		ui.y += dy;

		ui.vx = dx*0.5;
		ui.vy = dy*0.5;
	}

	mx = (float)x;
	my = (float)y;
}

void keydownfunc(unsigned char key, int x, int y)
{
	if (key == 27)
		exit(0);
}

void keyupfunc(unsigned char key, int x, int y)
{

}

void release()
{
	release(ui.base);
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);

	atexit(release);
	glutEnterGameMode();

	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		fprintf(stderr, "Error %s\n", glewGetErrorString(err));
		exit(1);
	}
	fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));

	if (glewGetExtension("GL_ARB_vertex_program"))
		fprintf(stdout, "Status: ARB vertex programs available.\n");

	if (glewGetExtension("GL_ARB_fragment_program"))
		fprintf(stdout, "Status: ARB fragment programs available.\n");

	if (glewIsSupported("GL_VERSION_1_4  GL_ARB_point_sprite"))
		fprintf(stdout, "Status: ARB point sprites available.\n");

	glutReshapeFunc(reshape);
	glutDisplayFunc(displayfunc);
	glutIdleFunc(displayfunc);
	glutPassiveMotionFunc(pmotionfunc);
	glutMotionFunc(motionfunc);
	glutMouseFunc(mousefunc);
	glutKeyboardFunc(keydownfunc);
	glutKeyboardUpFunc(keyupfunc);

	init();
	glutMainLoop();
	glutLeaveGameMode();
}
