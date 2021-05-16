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

	if (event->key() == Qt::Key_Space)
		isVR360 = !isVR360;
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


void QtRendererVideo::InitializeGL()
{
	capture = new VideoCapture();
	if (!capture->Open("test.flv", PIX_FMT_YUVJ420P))
		throw;
	
	quadModel = new Model("assets/quad.obj");
	sphereModel = new Model("assets/sphere.obj");

	if (capture->formatType == PIX_FMT_YUV420P || capture->formatType == PIX_FMT_YUVJ420P)
	{
		shader = new Shader("assets/vertexShader.glsl", "assets/VideoShader/fragmentShader_yuv.glsl");
		tex1 = new Texture2D(capture->width, capture->height, GL_LUMINANCE, GL_LUMINANCE, NULL);

		tex2 = new Texture2D(capture->width / 2, capture->height / 2, GL_LUMINANCE, GL_LUMINANCE, NULL);

		tex3 = new Texture2D(capture->width / 2, capture->height / 2, GL_LUMINANCE, GL_LUMINANCE, NULL);
	}
	else if (capture->formatType == PIX_FMT_YUV422P || capture->formatType == PIX_FMT_YUVJ422P)
	{
		shader = new Shader("assets/vertexShader.glsl", "assets/VideoShader/fragmentShader_yuv.glsl");
		tex1 = new Texture2D(capture->width, capture->height, GL_LUMINANCE, GL_LUMINANCE, NULL);

		tex2 = new Texture2D(capture->width / 2, capture->height, GL_LUMINANCE, GL_LUMINANCE, NULL);

		tex3 = new Texture2D(capture->width / 2, capture->height, GL_LUMINANCE, GL_LUMINANCE, NULL);
	}
	else if (capture->formatType == PIX_FMT_YUV444P || capture->formatType == PIX_FMT_YUVJ444P)
	{
		shader = new Shader("assets/vertexShader.glsl", "assets/VideoShader/fragmentShader_yuv.glsl");
		tex1 = new Texture2D(capture->width, capture->height, GL_LUMINANCE, GL_LUMINANCE, NULL);

		tex2 = new Texture2D(capture->width, capture->height, GL_LUMINANCE, GL_LUMINANCE, NULL);

		tex3 = new Texture2D(capture->width, capture->height, GL_LUMINANCE, GL_LUMINANCE, NULL);
	}

	

	glEnable(GL_DEPTH_TEST);
	//启用面剔除
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glPolygonMode(GL_FRONT, GL_FILL);
	
	CheckError();
}


void QtRendererVideo::Renderer()
{
	QMatrix4x4 projMat;
	projMat.perspective(60, width() / (float)height(), 0.1f, 100);

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
	
	Model *model = isVR360 ? sphereModel : quadModel;
	const float *_videMat = isVR360 ? camera.GetViewMat().constData() : QMatrix4x4().constData();
	const float *_projMat = isVR360 ? projMat.constData() : QMatrix4x4().constData();
	if (capture->formatType == PIX_FMT_YUV420P || capture->formatType == PIX_FMT_YUVJ420P)
	{
		tex1->UpdateTexture2D(frame->width, frame->height, frame->linesize[0], frame->data[0]);
		tex2->UpdateTexture2D(frame->width / 2, frame->height / 2, frame->linesize[1], frame->data[1]);
		tex3->UpdateTexture2D(frame->width / 2, frame->height / 2, frame->linesize[2], frame->data[2]);

		model->ApplyShader(shader);
		model->SetTexture2D("smp1", tex1);
		model->SetTexture2D("smp2", tex2);
		model->SetTexture2D("smp3", tex3);
		model->Draw(_videMat, _projMat);
	}
	else if (capture->formatType == PIX_FMT_YUV422P || capture->formatType == PIX_FMT_YUVJ422P)
	{
		tex1->UpdateTexture2D(frame->width, frame->height, frame->linesize[0], frame->data[0]);
		tex2->UpdateTexture2D(frame->width / 2, frame->height, frame->linesize[1], frame->data[1]);
		tex3->UpdateTexture2D(frame->width / 2, frame->height, frame->linesize[2], frame->data[2]);

		model->ApplyShader(shader);
		model->SetTexture2D("smp1", tex1);
		model->SetTexture2D("smp2", tex2);
		model->SetTexture2D("smp3", tex3);
		model->Draw(_videMat, _projMat);
	}
	else if (capture->formatType == PIX_FMT_YUV444P || capture->formatType == PIX_FMT_YUVJ444P)
	{
		tex1->UpdateTexture2D(frame->width, frame->height, frame->linesize[0], frame->data[0]);
		tex2->UpdateTexture2D(frame->width, frame->height, frame->linesize[1], frame->data[1]);
		tex3->UpdateTexture2D(frame->width, frame->height, frame->linesize[2], frame->data[2]);

		model->ApplyShader(shader);
		model->SetTexture2D("smp1", tex1);
		model->SetTexture2D("smp2", tex2);
		model->SetTexture2D("smp3", tex3);
		model->Draw(_videMat, _projMat);
	}
	SwapBuffers(dc);
}