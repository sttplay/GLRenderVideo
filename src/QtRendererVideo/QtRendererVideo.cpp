#include "QtRendererVideo.h"
#include <QResizeEvent>
#include "QtEvent.h"
#include "GLTools.h"
#include <QDebug>
#include <QTimer>
#include <QDateTime>
#include "VideoCapture.h"
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

bool QtRendererVideo::event(QEvent* event)
{
	if (event->type() == QtEvent::GL_Renderer)
	{
		//进行渲染
		Renderer();
	}
	return QWidget::event(event);
}



void QtRendererVideo::InitializeGL()
{
	capture = new VideoCapture();
	if (!capture->Open("test.mp4", PIX_FMT_RGBA))
		throw;
	shader = new Shader("assets/vertexShader.glsl", "assets/fragmentShader.glsl");
	shader2 = new Shader("assets/vertexShader.glsl", "assets/fragmentShader2.glsl");
	model = new Model("assets/teapot.obj");
	model2 = new Model("assets/quad.obj");
	/*glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);*/

	/*mesh = LoadObjModel("assets/teapot.obj", true);
	VBO = CreateGLBuffer(GL_ARRAY_BUFFER, GL_STATIC_DRAW, mesh->vertexCount * sizeof(Vertex), mesh->vertices);*/
	//glBindBuffer(GL_ARRAY_BUFFER, VBO);

	////启用顶点属性
	//glEnableVertexAttribArray(shader->posLocation);
	//glVertexAttribPointer(
	//	shader->posLocation, //顶点属性ID
	//	3, //几个数据构成一组
	//	GL_FLOAT, //数据类型
	//	GL_FALSE,
	//	sizeof(float) * 8, //步长
	//	(void*)(sizeof(float) * 0) //偏移量，第一组数据的起始位置
	//);
	////启用顶点属性
	//glEnableVertexAttribArray(shader->colorLocation);
	//glVertexAttribPointer(shader->colorLocation, 3, GL_FLOAT, GL_FALSE,sizeof(float) * 8, (void*)(sizeof(float) * 3));

	////启用顶点属性
	//glEnableVertexAttribArray(shader->texcoordLocation);
	//glVertexAttribPointer(shader->texcoordLocation, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 6));

	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glBindVertexArray(0);

	/*EBO = CreateGLBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, mesh->indexCount * sizeof(uint32_t), mesh->indices);*/

	QImage img = QImage("assets/tex1.jpg");
	tex1 = new Texture2D(img.width(), img.height(), GL_RGBA, GL_BGRA, img.bits());
	
	QImage img2 = QImage("assets/lollogo.png");
	tex2 = new Texture2D(img2.width(), img2.height(), GL_RGBA, GL_BGRA, img2.bits());

	

	glEnable(GL_DEPTH_TEST);
	//启用面剔除
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glPolygonMode(GL_FRONT, GL_FILL);
	
	CheckError();
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
	QMatrix4x4 projMat;
	projMat.perspective(45, width() / (float)height(), 0.1f, 100);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

retry:

	AVFrame *frame = nullptr;
	int ret = capture->Retrieve(frame);
	if (ret < 0)
		throw;
	if (ret == 0)
	{
		capture->Seek(0);
		goto retry;
	}
	qDebug() << frame->pts;
	{
		model->ApplyShader(shader2);
		model->SetTexture2D("smp1", tex1);
		model->SetTexture2D("smp2", tex2);
		model->SetPosition(0, 0, -2);
		model->SetRotation(30, 0, 1, 0);
		model->SetScale(2, 2, 2);
		model->Draw(camera.GetViewMat().constData(), projMat.constData());
	}

	{
		model2->ApplyShader(shader);
		model2->SetTexture2D("smp1", tex1);
		model2->SetTexture2D("smp2", tex2);
		model2->SetPosition(4, 0, -2);
		model2->SetRotation(-30, 0, 1, 0);
		model2->SetScale(2, 2, 2);
		model2->Draw(camera.GetViewMat().constData(), projMat.constData());
	}

	SwapBuffers(dc);
}