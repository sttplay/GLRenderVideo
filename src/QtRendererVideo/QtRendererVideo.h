#pragma once

#include <QtWidgets/QWidget>
#include "ui_QtRendererVideo.h"
#include <windows.h>
#include "GLTools.h"

class QtRendererVideo : public QWidget
{
	Q_OBJECT

public:
	QtRendererVideo(QWidget *parent = Q_NULLPTR);

	~QtRendererVideo();

	virtual QPaintEngine* paintEngine() const { return NULL; }

	virtual void resizeEvent(QResizeEvent *event);

	void GLUpdate();
private:

	bool CreateGLContext();

	void Renderer();

	bool event(QEvent* event);

	void InitializeGL();

private:
	Ui::QtRendererVideoClass ui;

	HDC dc;
	HGLRC rc;
	HWND hwnd;

	float vertices[32] = {
	//¶¥µã						ÑÕÉ«				texcoord uv
	-0.5f,	0.5f,	0.0f,	1.0f, 0.0f, 0.0f,	0.0f, 1.0f,		//left top
	-0.5f,	-0.5f,	0.0f,	0.0f, 1.0f, 0.0f,	0.0f, 0.0f,		//left bottom
	0.5f,	-0.5f,	0.0f,	0.0f, 0.0f, 1.0f,	1.0f, 0.0f,		//right bottom
	0.5f,	0.5f,	0.0f,	1.0f, 1.0f, 0.0f,	1.0f, 1.0f,		//right top
	};

	GLuint indices[6] = {
		0,1,2,
		0,2,3
	};

	GLuint program;
	GLuint VBO, VAO, EBO;
	GLuint tex1, tex2;
	GLuint smp1, smp2;
};
