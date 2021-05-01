#include "QtRendererVideo.h"
#include <assert.h>
#include <QResizeEvent>
#include "QtEvent.h"
#include "GLTools.h"
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "opengl32.lib")
QtRendererVideo::QtRendererVideo(QWidget *parent)
	: QWidget(parent, Qt::MSWindowsOwnDC)
{
	setAttribute(Qt::WA_PaintOnScreen);
	setAttribute(Qt::WA_NoSystemBackground);
	setAutoFillBackground(true);
	ui.setupUi(this);
	hwnd = (HWND)winId();
	CreateGLContext();
	wglMakeCurrent(dc, rc);
	assert(glewInit() == GLEW_OK);

	InitializeGL();
}

QtRendererVideo::~QtRendererVideo()
{
	wglMakeCurrent(NULL, NULL);
	if (rc)
		wglDeleteContext(rc);
	if (dc)
		ReleaseDC(hwnd, dc);
}

void QtRendererVideo::resizeEvent(QResizeEvent *event)
{
	glViewport(0, 0, event->size().width(), event->size().height());
	GLUpdate();
}

void QtRendererVideo::GLUpdate()
{
	QApplication::postEvent(this, new QtEvent(QtEvent::GL_Renderer));
}

bool QtRendererVideo::CreateGLContext()
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

void QtRendererVideo::Renderer()
{
	static float redv = 0;
	redv += 0.01;
	if (redv > 1)
		redv = 0;
	glClearColor(redv, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	SwapBuffers(dc);
}

void QtRendererVideo::InitializeGL()
{
	CompileShader(GL_VERTEX_SHADER, "assets/vertexShader.glsl");
	CompileShader(GL_FRAGMENT_SHADER, "assets/fragmentShader.glsl");
	glGenBuffers(
		1, //创建VBO的个数
		&VBO
	);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	int size = sizeof(vertices);
	glBufferData(
		GL_ARRAY_BUFFER, //缓冲类型
		size, //数据量的大小
		vertices,//顶点数据
		GL_STATIC_DRAW
	);
	//GL_STATIC_DRAW//数据几乎不会改变
	//GL_DYNAMIC_DRAW 数据可能会发生改变
	//GL_STREAM_DRAW 每次绘制数据都会发生改变
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	assert(!glGetError());
}

GLuint QtRendererVideo::CompileShader(GLenum shaderType, const char *url)
{
	char *shaderCode = LoadFileContext(url);
	char *shaderTypeStr = "Vertex Shader";
	if (shaderType == GL_FRAGMENT_SHADER)
	{
		shaderTypeStr = "Fragment Shader";
	}
	GLuint shader = glCreateShader(shaderType);
	if (!shader)
		throw;
	glShaderSource(shader, 1, &shaderCode, NULL);
	glCompileShader(shader);
	GLint success = GL_TRUE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		char infolog[1024];
		GLsizei logLen = 0;
		glGetShaderInfoLog(shader, sizeof(infolog), &logLen, infolog);
		printf("[ERROR] Compile %s error: %s", shaderTypeStr, infolog);
		glDeleteShader(shader);
		throw;
	}
	printf("Compile Success");
	delete shaderCode;
	return shader;
}

bool QtRendererVideo::event(QEvent* event)
{
	if (event->type() == QtEvent::GL_Renderer)
	{
		//进行渲染
		Renderer();
	}
	return QWidget::event(event);
}