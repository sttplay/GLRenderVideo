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
	//ָ���ṹ��汾�ţ�����Ϊ1
	pfd.nVersion = 1;
	pfd.cColorBits = 32;
	//������Ȼ���
	pfd.cDepthBits = 24;
	//����ģ�建����λ��
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


void QtRendererVideo::InitializeGL()
{
	GLuint program = CreateGPUProgram("assets/vertexShader.glsl", "assets/fragmentShader.glsl");
	glUseProgram(program);
	GLint posLocation = glGetAttribLocation(program, "pos");
	GLint colorLocation = glGetAttribLocation(program, "color");

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glGenBuffers(
		1, //����VBO�ĸ���
		&VBO
	);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	int size = sizeof(vertices);
	glBufferData(
		GL_ARRAY_BUFFER, //��������
		size, //�������Ĵ�С
		vertices,//��������
		GL_STATIC_DRAW
	);
	//GL_STATIC_DRAW//���ݼ�������ı�
	//GL_DYNAMIC_DRAW ���ݿ��ܻᷢ���ı�
	//GL_STREAM_DRAW ÿ�λ������ݶ��ᷢ���ı�

	//���ö�������
	glEnableVertexAttribArray(posLocation);
	glVertexAttribPointer(
		posLocation, //��������ID
		3, //�������ݹ���һ��
		GL_FLOAT, //��������
		GL_FALSE,
		sizeof(float) * 6, //����
		(void*)(sizeof(float) * 0) //ƫ��������һ�����ݵ���ʼλ��
	);
	//���ö�������
	glEnableVertexAttribArray(colorLocation);
	glVertexAttribPointer(colorLocation, 3, GL_FLOAT, GL_FALSE,sizeof(float) * 6, (void*)(sizeof(float) * 3));

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//�������޳�
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_FRONT);
	glPolygonMode(GL_FRONT, GL_FILL);
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

GLuint QtRendererVideo::CreateGPUProgram(const char* vs, const char *fs)
{
	GLuint vsShader = CompileShader(GL_VERTEX_SHADER, vs);
	GLuint fsShader = CompileShader(GL_FRAGMENT_SHADER, fs);

	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vsShader);
	glAttachShader(shaderProgram, fsShader);
	glLinkProgram(shaderProgram);

	GLint success = GL_TRUE;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		char infolog[1024];
		GLsizei logLen = 0;
		glGetShaderInfoLog(shaderProgram, sizeof(infolog), &logLen, infolog);
		printf("[ERROR] Link error: %s", infolog);
		glDeleteProgram(shaderProgram);
		throw;
	}
	printf("Line shader program success\n");
	glDetachShader(shaderProgram, vsShader);
	glDetachShader(shaderProgram, fsShader);
	glDeleteShader(vsShader);
	glDeleteShader(fsShader);
	return shaderProgram;
}

bool QtRendererVideo::event(QEvent* event)
{
	if (event->type() == QtEvent::GL_Renderer)
	{
		//������Ⱦ
		Renderer();
	}
	return QWidget::event(event);
}

void QtRendererVideo::Renderer()
{

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindVertexArray(VAO);
	//glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
	SwapBuffers(dc);
}