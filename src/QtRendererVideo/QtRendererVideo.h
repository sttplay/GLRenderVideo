#pragma once

#include <QtWidgets/QWidget>
#include "ui_QtRendererVideo.h"
#include <windows.h>

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

private:
	Ui::QtRendererVideoClass ui;

	HDC dc;
	HGLRC rc;
	HWND hwnd;


};
