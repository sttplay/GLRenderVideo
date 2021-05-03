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

	GLuint CompileShader(GLenum shaderType, const char *url);

	GLuint CreateGPUProgram(const char* vs, const char *fs);
private:
	Ui::QtRendererVideoClass ui;

	HDC dc;
	HGLRC rc;
	HWND hwnd;

	float vertices[24] = {
	//¶¥µã						ÑÕÉ«
	-0.5f,	0.5f,	0.0f,	1.0f, 0.0f, 0.0f,	//left top
	-0.5f,	-0.5f,	0.0f,	0.0f, 1.0f, 0.0f,	//left bottom
	0.5f,	-0.5f,	0.0f,	0.0f, 0.0f, 1.0f,	//right bottom
	0.5f,	0.5f,	0.0f,	1.0f, 1.0f, 0.0f	//right top
	};

	GLuint indices[6] = {
		0,1,2,
		0,2,3
	};

	GLuint VBO, VAO, EBO;
};
