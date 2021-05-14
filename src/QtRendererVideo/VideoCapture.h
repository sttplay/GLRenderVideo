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
	 * @param url �ļ�·��
	 * @param fmt ������ظ�ʽ
	 * @return �ɹ�����true ʧ�ܷ���false
	 */
	bool Open(const char *url, PixelFormatType fmt = PIX_FMT_AUTO);

	/*
	 * �رղ��ͷ���Դ
	 */
	void Close();

	/*
	 * ��ȡһ֡��Ƶ����
	 * @param outFrame ���֡����, ԭʼ���ظ�ʽ
	 * @return -1 ����-2û�д򿪣� 0����ȡ����β��1��ȡ�ɹ�
	 */
	int GrabFrame(AVFrame* &outFrame);

	/*
	 * ��ȡһ֡��Ƶ���ݲ�ת����Ŀ�����ظ�ʽ
	 * @param outFrame ���֡���ݣ�Ŀ�����ظ�ʽ��open��������
	 * @return -1 ���� -2û�д򿪣� 0����ȡ����β��1��ȡ�ɹ�
	 */
	int Retrieve(AVFrame* &outFrame);

	/*
	 * Seek��Ŀ��λ������Ĺؼ�֡
	 * @param percentage Ŀ��λ�ã��ٷֱ�[0 ~ 1)
	 * @return �ɹ�����true
	 */
	bool Seek(double percentage);

public:
	PixelFormatType formatType = PIX_FMT_AUTO;
	int width;
	int height;
private:
	
	//��Ƶ������
	int videoStreamIndex = -1;
	//��Ƶ��
	AVStream *videoStream = nullptr;
	//�ӷ�װ������
	AVFormatContext *ic = nullptr;
	//������������
	AVCodecContext *codecCtx = nullptr;

	AVFrame *frame = nullptr;
	AVFrame *sws_frame = nullptr;

	//���ظ�ʽת��������
	SwsContext *swsCtx = nullptr;

	bool isOpened = false;
};

