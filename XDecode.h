#pragma once
#include <mutex>
struct AVCodecParameters;
struct AVCodecContext;
struct AVFrame;
struct AVPacket;
extern void XFreePacket(AVPacket **pkt);
class XDecode
{
public:
	bool isAudio = false;
	//��ǰ���뵽��pts
	long long pts = 0;

	//�򿪽�����,�����Ƿ�ɹ������ͷſռ䣬����Ƶͨ��
	virtual bool Open(AVCodecParameters *para);

	//���͵������̣߳����ܳ¹��������pkt�ռ䣬�����ý�����ݿռ�
	virtual bool Send(AVPacket *pkt);
	//��ȡ�������ݣ�һ��send������Ҫ���Recv,��ȡ�����е�����Send NULL��Recv�ж��
	//ÿ�θ�ֵһ�ݣ��ɵ������ͷ�av_frame_free
	virtual AVFrame* Recv();

	virtual void Close();
	virtual void Clear();

	XDecode();
	virtual ~XDecode();

	protected:
		AVCodecContext *codec = 0;
		//ͷ�ļ��о�����Ҫʹ�������ռ䣬ͷ�ļ����ɿأ���������˭ʹ�ã�������ɳ�ͻ
		//�����ռ��ֹͬ��������ͻ��cpp�ļ����Լ��ɿصĿ���ʹ�������ռ�
		std::mutex mux;
	
};

