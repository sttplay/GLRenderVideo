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


void QtRendererVideo::InitializeGL()
{
	program = CreateGPUProgram("assets/vertexShader.glsl", "assets/fragmentShader.glsl");

	GLint posLocation = glGetAttribLocation(program, "pos");
	GLint colorLocation = glGetAttribLocation(program, "color");
	GLint texcoordLocation = glGetAttribLocation(program, "texcoord");

	smp1 = glGetUniformLocation(program, "smp1");
	smp2 = glGetUniformLocation(program, "smp2");

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
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

	//启用顶点属性
	glEnableVertexAttribArray(posLocation);
	glVertexAttribPointer(
		posLocation, //顶点属性ID
		3, //几个数据构成一组
		GL_FLOAT, //数据类型
		GL_FALSE,
		sizeof(float) * 8, //步长
		(void*)(sizeof(float) * 0) //偏移量，第一组数据的起始位置
	);
	//启用顶点属性
	glEnableVertexAttribArray(colorLocation);
	glVertexAttribPointer(colorLocation, 3, GL_FLOAT, GL_FALSE,sizeof(float) * 8, (void*)(sizeof(float) * 3));

	//启用顶点属性
	glEnableVertexAttribArray(texcoordLocation);
	glVertexAttribPointer(texcoordLocation, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 6));

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glGenTextures(1, &tex1);
	glBindTexture(GL_TEXTURE_2D, tex1);
	//设置环绕方式
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//设置过滤方式
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	QImage img = QImage("assets/tex1.jpg");
	glTexImage2D(
		GL_TEXTURE_2D, //纹理类型
		0, //多级渐远纹理级别
		GL_RGBA, //纹理的格式
		img.width(),
		img.height(),
		0,
		GL_BGRA, //数据的格式
		GL_UNSIGNED_BYTE,
		img.bits()
	);
	glGenerateMipmap(GL_TEXTURE_2D);

	glGenTextures(1, &tex2);
	glBindTexture(GL_TEXTURE_2D, tex2);
	//设置环绕方式
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//设置过滤方式
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	QImage img2 = QImage("assets/lollogo.png");
	glTexImage2D(
		GL_TEXTURE_2D, //纹理类型
		0, //多级渐远纹理级别
		GL_RGBA, //纹理的格式
		img2.width(),
		img2.height(),
		0,
		GL_BGRA, //数据的格式
		GL_UNSIGNED_BYTE,
		img2.bits()
	);
	glGenerateMipmap(GL_TEXTURE_2D);

	//启用面剔除
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
		//进行渲染
		Renderer();
	}
	return QWidget::event(event);
}

void QtRendererVideo::Renderer()
{

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(program);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex1);
	glUniform1i(smp1, 0);

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, tex2);
	glUniform1i(smp2, 5);

	glBindVertexArray(VAO);
	//glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
	SwapBuffers(dc);
}