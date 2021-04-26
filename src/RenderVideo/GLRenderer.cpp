#include "GLRenderer.h"
#include <assert.h>
//#define GLEW_STATIC
#include <GL/glew.h>
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "opengl32.lib")
GLRenderer::GLRenderer(HWND hwnd, int width, int height)
{
	this->hwnd = hwnd;
	CreateGLContext();
	wglMakeCurrent(dc, rc);
	assert(glewInit() == GLEW_OK);

	Resize(width, height);
}

GLRenderer::~GLRenderer()
{
	wglMakeCurrent(NULL, NULL);
	if (rc)
		wglDeleteContext(rc);
	if (dc)
		ReleaseDC(hwnd, dc);
}

void GLRenderer::Renderer()
{
	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	SwapBuffers(dc);
}

void GLRenderer::Resize(int width, int height)
{
	glViewport(0, 0, width, height);
}

bool GLRenderer::CreateGLContext()
{
	dc = GetDC(hwnd);

	PIXELFORMATDESCRIPTOR pfd;
	ZeroMemory(&pfd, sizeof(pfd));
	pfd.nSize = sizeof(pfd);
	//指定结构体版本号，设置为1
	pfd.nVersion = 1;
	pfd.cColorBits = 32;
	//设置深度缓冲
	pfd.cDepthBits = 24;
	//设置模板缓冲区位深
	pfd.cStencilBits = 8;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;

	int format = 0;
	format = ChoosePixelFormat(dc, &pfd);
	if (!format)
	{
		throw;
	}
	SetPixelFormat(dc, format, &pfd);
	rc = wglCreateContext(dc);
	return true;
}
