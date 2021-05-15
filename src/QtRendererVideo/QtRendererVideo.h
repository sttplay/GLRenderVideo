#pragma once

#include <QtWidgets/QWidget>
#include "ui_QtRendererVideo.h"
#include <windows.h>
#include "Shader.h"
#include "Camera.h"
#include "Texture2D.h"
#include "Model.h"

class QtRendererVideo : public QWidget
{
	Q_OBJECT

public:
	QtRendererVideo(QWidget *parent = Q_NULLPTR);

	~QtRendererVideo();

	virtual QPaintEngine* paintEngine() const { return NULL; }

	virtual void resizeEvent(QResizeEvent *event);

	void GLUpdate();

	void showEvent(QShowEvent *event);

	void keyPressEvent(QKeyEvent *event);

	void keyReleaseEvent(QKeyEvent *event);

	void mousePressEvent(QMouseEvent *event);

	void mouseMoveEvent(QMouseEvent *event);

private:

	bool CreateGLContext();

	void Renderer();

	bool event(QEvent* event);

	void InitializeGL();

private slots:
	void Tick();
private:
	Ui::QtRendererVideoClass ui;

	HDC dc;
	HGLRC rc;
	HWND hwnd;


	Shader *shader = NULL /**shader2 = NULL*/;
	//GLuint VBO, VAO, EBO;
	Texture2D* tex1, *tex2, *tex3;
	Model *quadModel = NULL, *sphereModel = NULL;

	Camera camera;

	bool isPressW = false, isPressS = false, isPressA = false, isPressD = false, isPressQ = false, isPressE = false;
	float speed = 1.0f;
	float rotateSpeed = 0.2f;

	QPoint lastPoint;

	//struct Mesh *mesh = NULL;

	class VideoCapture *capture = nullptr;

	bool isVR360 = false;
};
