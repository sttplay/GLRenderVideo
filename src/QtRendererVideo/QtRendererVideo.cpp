#include "QtRendererVideo.h"
#include <QResizeEvent>
#include "QtEvent.h"
#include "GLTools.h"
#include <QDebug>
#include <QTimer>
#include <QDateTime>
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
	if (glewInit() != GLEW_OK)
	{
		qDebug() << "glewInit failed!";
		throw;
	}

	QTimer *timer = new QTimer(parent);
	connect(timer, &QTimer::timeout, this, &QtRendererVideo::Tick);
	timer->start(20);
	camera.SetRotation(0, 180, 0);
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

void QtRendererVideo::showEvent(QShowEvent *event)
{
	GLUpdate();
}

void QtRendererVideo::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_W)
		isPressW = true;
	if (event->key() == Qt::Key_S)
		isPressS = true;
	if (event->key() == Qt::Key_A)
		isPressA = true;
	if (event->key() == Qt::Key_D)
		isPressD = true;
	if (event->key() == Qt::Key_Q)
		isPressQ = true;
	if (event->key() == Qt::Key_E)
		isPressE = true;
}

void QtRendererVideo::keyReleaseEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_W)
		isPressW = false;
	if (event->key() == Qt::Key_S)
		isPressS = false;
	if (event->key() == Qt::Key_A)
		isPressA = false;
	if (event->key() == Qt::Key_D)
		isPressD = false;
	if (event->key() == Qt::Key_Q)
		isPressQ = false;
	if (event->key() == Qt::Key_E)
		isPressE = false;
}

void QtRendererVideo::mousePressEvent(QMouseEvent *event)
{
	lastPoint = event->pos();
}

void QtRendererVideo::mouseMoveEvent(QMouseEvent *event)
{
	QPoint delta = event->pos() - lastPoint;
	lastPoint = event->pos();
	camera.Rotate(delta.y() * rotateSpeed, delta.x() * rotateSpeed, 0);
	GLUpdate();
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

bool QtRendererVideo::event(QEvent* event)
{
	if (event->type() == QtEvent::GL_Renderer)
	{
		//������Ⱦ
		Renderer();
	}
	return QWidget::event(event);
}



void QtRendererVideo::InitializeGL()
{
	program = CreateGPUProgram("assets/vertexShader.glsl", "assets/fragmentShader.glsl");

	GLint posLocation = glGetAttribLocation(program, "pos");
	GLint colorLocation = glGetAttribLocation(program, "color");
	GLint texcoordLocation = glGetAttribLocation(program, "texcoord");

	smp1 = glGetUniformLocation(program, "smp1");
	smp2 = glGetUniformLocation(program, "smp2");

	modelLocation = glGetUniformLocation(program, "modelMat");
	viewLocation = glGetUniformLocation(program, "viewMat");
	projLocation = glGetUniformLocation(program, "projMat");

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	VBO = CreateGLBuffer(GL_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(vertices), vertices);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	//���ö�������
	glEnableVertexAttribArray(posLocation);
	glVertexAttribPointer(
		posLocation, //��������ID
		3, //�������ݹ���һ��
		GL_FLOAT, //��������
		GL_FALSE,
		sizeof(float) * 8, //����
		(void*)(sizeof(float) * 0) //ƫ��������һ�����ݵ���ʼλ��
	);
	//���ö�������
	glEnableVertexAttribArray(colorLocation);
	glVertexAttribPointer(colorLocation, 3, GL_FLOAT, GL_FALSE,sizeof(float) * 8, (void*)(sizeof(float) * 3));

	//���ö�������
	glEnableVertexAttribArray(texcoordLocation);
	glVertexAttribPointer(texcoordLocation, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 6));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	EBO = CreateGLBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(indices), indices);

	QImage img = QImage("assets/tex1.jpg");
	tex1 = CreateGLTexture(GL_TEXTURE_2D, img.width(), img.height(), GL_RGBA, GL_BGRA, img.bits());
	
	QImage img2 = QImage("assets/lollogo.png");
	tex2 = CreateGLTexture(GL_TEXTURE_2D, img2.width(), img2.height(), GL_RGBA, GL_BGRA, img2.bits());

	

	//�������޳�
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glPolygonMode(GL_FRONT, GL_FILL);
	GLenum errorCode = glGetError();
	if (errorCode)
	{
		qDebug() << "glGetError:" << errorCode;
		throw;
	}
}


void QtRendererVideo::Tick()
{
	static long long lastts = 0;
	if (lastts == 0) lastts = QDateTime::currentMSecsSinceEpoch();
	float dt = QDateTime::currentMSecsSinceEpoch() - lastts;
	lastts = QDateTime::currentMSecsSinceEpoch();
	dt /= 1000;

	if (isPressW)
		camera.Translate(0, 0, speed * dt);
	if (isPressS)
		camera.Translate(0, 0, -speed * dt);
	if (isPressA)
		camera.Translate(-speed * dt, 0, 0);
	if (isPressD)
		camera.Translate(speed * dt, 0, 0);
	if (isPressE)
		camera.Translate(0, speed * dt, 0);
	if (isPressQ)
		camera.Translate(0, -speed * dt, 0);
	GLUpdate();
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

	QMatrix4x4 modelMat;
	QMatrix4x4 projMat;

	modelMat.translate(0, 0, -2);
	modelMat.rotate(30, QVector3D(0, 0, 1));
	modelMat.scale(2, 1, 1);

	
	projMat.perspective(45, width() / (float)height(), 0.1f, 100);

	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, modelMat.constData());
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, camera.GetViewMat().constData());
	glUniformMatrix4fv(projLocation, 1, GL_FALSE, projMat.constData());
	glBindVertexArray(VAO);
	//glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
	SwapBuffers(dc);
}