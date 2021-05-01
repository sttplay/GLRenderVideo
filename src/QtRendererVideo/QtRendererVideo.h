#pragma once

#include <QtWidgets/QWidget>
#include "ui_QtRendererVideo.h"
#include <windows.h>
#include <GL/eglew.h>

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

	float vertices[9] = {
	0.0f,	0.5f,	0.0f,
	-0.5f,	-0.5f,	0.0f,
	0.5f,	-0.5f,	0.0f
	};

	GLuint VBO;
};
