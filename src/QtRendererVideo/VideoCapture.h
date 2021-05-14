#pragma once
extern "C"
{
#include <libavutil/frame.h>
}
enum PixelFormatType
{
	PIX_FMT_AUTO = -1,

	PIX_FMT_YUV420P = 0,
	PIX_FMT_YUYV422 = 1,
	PIX_FMT_UYVY422 = 15,
	PIX_FMT_YUV422P = 4,
	PIX_FMT_YUV444P = 5,

	PIX_FMT_GRAY = 8,

	PIX_FMT_YUVJ420P = 12,
	PIX_FMT_YUVJ422P = 13,
	PIX_FMT_YUVJ444P = 14,

	PIX_FMT_NV12 = 23,
	PIX_FMT_NV21 = 24,

	PIX_FMT_RGB = 2,
	PIX_FMT_BGR = 3,

	PIX_FMT_ARGB = 25,
	PIX_FMT_RGBA = 26,
	PIX_FMT_ABGR = 27,
	PIX_FMT_BGRA = 28,
};
struct AVStream;
struct AVFormatContext;
struct AVCodecContext;
struct SwsContext;
class VideoCapture
{
public:
	VideoCapture();
	~VideoCapture();

	/* 
	 * @param url 文件路径
	 * @param fmt 输出像素格式
	 * @return 成功返回true 失败返回false
	 */
	bool Open(const char *url, PixelFormatType fmt = PIX_FMT_AUTO);

	/*
	 * 关闭并释放资源
	 */
	void Close();

	/*
	 * 读取一帧视频数据
	 * @param outFrame 输出帧数据, 原始像素格式
	 * @return -1 错误，-2没有打开， 0，获取到结尾，1获取成功
	 */
	int GrabFrame(AVFrame* &outFrame);

	/*
	 * 读取一帧视频数据并转换到目标像素格式
	 * @param outFrame 输出帧数据，目标像素格式，open函数设置
	 * @return -1 错误 -2没有打开， 0，获取到结尾，1获取成功
	 */
	int Retrieve(AVFrame* &outFrame);

	/*
	 * Seek到目标位置相近的关键帧
	 * @param percentage 目标位置，百分比[0 ~ 1)
	 * @return 成功返回true
	 */
	bool Seek(double percentage);

public:
	PixelFormatType formatType = PIX_FMT_AUTO;
	int width;
	int height;
private:
	
	//视频流索引
	int videoStreamIndex = -1;
	//视频流
	AVStream *videoStream = nullptr;
	//接封装上下文
	AVFormatContext *ic = nullptr;
	//解码器上下文
	AVCodecContext *codecCtx = nullptr;

	AVFrame *frame = nullptr;
	AVFrame *sws_frame = nullptr;

	//像素格式转换上下文
	SwsContext *swsCtx = nullptr;

	bool isOpened = false;
};

