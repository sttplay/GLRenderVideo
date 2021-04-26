#pragma once
#include <windows.h>
class GLRenderer
{
public:
	GLRenderer(HWND hwnd, int width, int height);
	~GLRenderer();

	void Renderer();

	void Resize(int width, int height);
private:
	bool CreateGLContext();

private:
	HDC dc;
	HGLRC rc;
	HWND hwnd;
};

