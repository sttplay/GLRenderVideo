#include "VideoCapture.h"
extern "C"
{
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "swscale")
static char errorbuffer[AV_ERROR_MAX_STRING_SIZE];
char* ffmepgerror(int errCode)
{
	return av_make_error_string(errorbuffer, AV_ERROR_MAX_STRING_SIZE, errCode);
}

VideoCapture::VideoCapture()
{
	avformat_network_init();
}

VideoCapture::~VideoCapture()
{
	Close();
}

bool VideoCapture::Open(const char *url, PixelFormatType fmt)
{
	Close();
	isOpened = false;
	formatType = fmt;
	//1.�򿪽��װ������
	int ret = avformat_open_input(
		&ic,  //���װ������
		url, //�ļ�·��
		NULL, //ָ�������ʽ h264,h265, ֮��ģ� ����NULL���Զ����
		NULL); //���ò������ֵ�
	if (ret != 0)
	{
		printf("%s\n", ffmepgerror(ret));
		return false;
	}
	//2.��ȡ�ļ���Ϣ
	ret = avformat_find_stream_info(ic, NULL);
	if (ret < 0)
	{
		printf("%s\n", ffmepgerror(ret));
		return false;
	}

	//3.��ȡĿ��������
	for (unsigned int i = 0; i < ic->nb_streams; i++)
	{
		AVStream *stream = ic->streams[i];
		if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			if (!videoStream)
			{
				videoStreamIndex = i;
				videoStream = stream;
			}
			else
				stream->discard = AVDISCARD_ALL;
		}
		else
			stream->discard = AVDISCARD_ALL;
	}

	//4.���ҽ�����
	AVCodec *codec = avcodec_find_decoder(videoStream->codecpar->codec_id);
	if (!codec)
	{
		printf("can't find codec, codec id:%d\n", videoStream->codecpar->codec_id);
		return false;
	}

	//5.����������������
	if (!(codecCtx = avcodec_alloc_context3(codec)))
	{
		printf("avcodec_alloc_context3 failed,\n");
		return false;
	}

	//6.�����������Ʊ����������������������������
	if ((ret = avcodec_parameters_to_context(codecCtx, videoStream->codecpar)) < 0) {
		printf("Failed to copy %s codec parameters to decoder context\n",
			av_get_media_type_string(videoStream->codecpar->codec_type));
		return false;
	}

	//7. �򿪽����������� */
	if ((ret = avcodec_open2(codecCtx, codec, nullptr)) < 0) {
		printf("Failed to open %s codec\n",
			av_get_media_type_string(videoStream->codecpar->codec_type));
		return false;
	}
	//����һ��frame���ս���֮���֡����
	frame = av_frame_alloc();
	isOpened = true;
	width = videoStream->codecpar->width;
	height = videoStream->codecpar->height;

	//����Ƿ�֧�ֵ�ǰ��Ƶ�����ظ�ʽ
	AVPixelFormat pixfmt = (AVPixelFormat)videoStream->codecpar->format;
	bool support = pixfmt == PIX_FMT_YUV420P ||
		pixfmt == PIX_FMT_YUYV422 ||
		pixfmt == PIX_FMT_YUV422P ||
		pixfmt == PIX_FMT_YUV444P ||
		pixfmt == PIX_FMT_GRAY ||
		pixfmt == PIX_FMT_YUVJ420P ||
		pixfmt == PIX_FMT_YUVJ422P ||
		pixfmt == PIX_FMT_YUVJ444P ||
		pixfmt == PIX_FMT_NV12 ||
		pixfmt == PIX_FMT_NV21 ||
		pixfmt == PIX_FMT_RGB ||
		pixfmt == PIX_FMT_BGR ||
		pixfmt == PIX_FMT_ARGB ||
		pixfmt == PIX_FMT_RGBA ||
		pixfmt == PIX_FMT_ABGR ||
		pixfmt == PIX_FMT_BGRA;

	//��֧�ָ���formatType��������
	if (formatType == PIX_FMT_AUTO)
		formatType = support ? (PixelFormatType)pixfmt : PIX_FMT_BGRA;

	return true;
}

void VideoCapture::Close()
{
	if (ic)
		avformat_close_input(&ic);
	if (codecCtx)
		avcodec_free_context(&codecCtx);
	if (swsCtx)
		sws_freeContext(swsCtx);
	swsCtx = nullptr;
	if (frame)
		av_frame_free(&frame);
	if (sws_frame)
		av_frame_free(&frame);
}

int VideoCapture::GrabFrame(AVFrame* &outFrame)
{
	if (!isOpened)
		return -2;
	int ret = 0;
	//����AVPacket�����洢ѹ����֡����
	AVPacket pkt;
	av_init_packet(&pkt);
	outFrame = nullptr;
	do
	{
		ret = avcodec_receive_frame(codecCtx, frame);
		outFrame = frame;
		if (ret >= 0)
			return 1;
		//��ȡ����β
		if (ret == AVERROR_EOF)
		{
			outFrame = nullptr;
			return 0;
		}
		else if (ret != AVERROR(EAGAIN))
		{
			printf("Error submitting a packet for decoding (%s)\n", ffmepgerror(ret));
			return -1;
		}

		//��ȡһ֡ѹ������
		ret = av_read_frame(ic, &pkt);
		if (ret != AVERROR_EOF && pkt.stream_index != videoStreamIndex)
		{
			av_packet_unref(&pkt);
			continue;
		}
		if (ret < 0)
		{
			//�ж��Ƿ��ȡ����β����ȡ����βseek����һ֡
			if (ret == AVERROR_EOF)
				av_init_packet(&pkt);
			else
			{
				printf("av_read_frame error:%s\n", ffmepgerror(ret));
				return -1;
			}
		}

		//ѹ��֡���ݾݷ��͵������߳�
		ret = avcodec_send_packet(codecCtx, &pkt);
		av_packet_unref(&pkt);
		if (ret < 0) {
			printf("Error submitting a packet for decoding (%s)\n", ffmepgerror(ret));
			return -1;
		}

	} while (true);

	return -1;
}

int VideoCapture::Retrieve(AVFrame* &outFrame)
{
	AVFrame *srcFrame = nullptr;
	int ret = GrabFrame(srcFrame);
	if (ret > 0)
	{
		//�жϽ�����������ظ�ʽ��Ŀ�����ظ�ʽ�Ƿ���ͬ
		if (srcFrame->format == formatType)
			outFrame = srcFrame;
		else
		{
			//���ظ�ʽת��
			if (!sws_frame)
			{
				sws_frame = av_frame_alloc();
				sws_frame->format = (AVPixelFormat)formatType;
				sws_frame->width = srcFrame->width;
				sws_frame->height = srcFrame->height;
				int ret = av_frame_get_buffer(sws_frame, 0);
				if (ret)
				{
					printf("av_frame_get_buffer failed: %s\n", ffmepgerror(ret));
					return -1;
				}
			}

			swsCtx = sws_getCachedContext(
				swsCtx,									//��NULL���´��� �����֮ǰ��ͬ��ֱ�ӷ���
				srcFrame->width, srcFrame->height,		// ����Ŀ��
				(AVPixelFormat)srcFrame->format,		// ����ĸ�ʽ
				sws_frame->width, sws_frame->height,	// ����Ŀ��
				(AVPixelFormat)sws_frame->format,		// ����ĸ�ʽ
				SWS_POINT,								// �ߴ�任���㷨
				0, 0, 0
			);

			int ret = sws_scale(
				swsCtx,
				srcFrame->data,				//��������
				srcFrame->linesize,			//�����д�С ���Ƕ���
				0,							//��0��ʼ
				srcFrame->height,			//����ĸ߶�
				sws_frame->data,			//��������� 
				sws_frame->linesize			//����Ĵ�С
			);
			if (ret <= 0)
			{
				printf("sws_scale failed: %s", ffmepgerror(ret));
				return -1;
			}
			sws_frame->pts = srcFrame->pts;
			sws_frame->colorspace = srcFrame->colorspace;
			sws_frame->color_range = srcFrame->color_range;
			outFrame = sws_frame;
		}

	}
	else
		outFrame = nullptr;
	return ret;
}

bool VideoCapture::Seek(double percentage)
{
	if (!isOpened)
		return false;
	int64_t ts = ic->duration * percentage;
	int ret = av_seek_frame(ic, -1, ts, AVSEEK_FLAG_FRAME);
	if (ret < 0)
	{
		printf("Seek error: %s", ffmepgerror(ret));
		return false;
	}
	avcodec_flush_buffers(codecCtx);
	return true;
}
