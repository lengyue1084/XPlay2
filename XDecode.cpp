#include "XDecode.h"
#include <iostream>
using namespace std;
extern "C" {
#include "libavcodec/avcodec.h"
}
XDecode::XDecode() {
};
XDecode::~XDecode() {
};
 void XDecode::Close()
{
	 mux.lock();
	 if (codec) {//�رյ�ʱ��������뻺����
		 avcodec_close(codec);
		 avcodec_free_context(&codec);
	 }
	 mux.unlock();
}

 void XDecode::Clear() {
	 mux.lock();
	 //������뻺��
	 if (codec) avcodec_flush_buffers(codec);
	 mux.unlock();

}

bool XDecode::Open(AVCodecParameters *para)
{
	//Close();
	if (!para) return false;
	Close();
	//////////////////////////////////////////////////////////
	///��Ƶ��������
	///�ҵ���Ƶ������
	AVCodec *vcodec = avcodec_find_decoder(para->codec_id);
	if (!vcodec)
	{
		avcodec_parameters_free(&para);
		cout << "can't find the codec id " << para->codec_id << endl;
		return false;
	}
	cout << "find the AVCodec " << para->codec_id << endl;

	mux.lock();//�˴���ʼ���ʹ������
	//AVCodecContext *codec = avcodec_alloc_context3(vcodec);
	 codec = avcodec_alloc_context3(vcodec);

	///���ý����������Ĳ���
	int re = avcodec_parameters_to_context(codec, para);
	if (re < 0) {
		cout << "avcodec_parameters_to_context  failed! :" << endl;
		mux.unlock();
		return false;
	}
	//���߳̽���
	codec->thread_count = 8;

	///�򿪽�����������
	 re = avcodec_open2(codec, 0, 0);
	if (re != 0)
	{
		avcodec_free_context(&codec);
		mux.unlock();
		char buf[1024] = { 0 };
		av_strerror(re, buf, sizeof(buf) - 1);
		cout << "avcodec_open2  failed! :" << buf << endl;

		return false;
	}
	mux.unlock();
	cout << "video avcodec_open2 success!" << endl;
	return true;
};

//���͵������̣߳����ܳɹ��������pkt�ռ䣬�����ý�����ݿռ�
bool XDecode::Send(AVPacket *pkt) {

	// �̴�
	if (!pkt || pkt->size <= 0 || !pkt->data) return false;
	mux.lock();
	if (!codec) { //���������������
		mux.unlock();
		return false;
	}
	int re = avcodec_send_packet(codec,pkt);
	mux.unlock();
	av_packet_free(&pkt);//������ǳɶԵ�
	if (re != 0) return false;
	return true;
}


//��ȡ�������ݣ�һ��send������Ҫ���Recv,��ȡ�����е�����Send NULL��Recv�ж��
//ÿ�θ�ֵһ�ݣ��ɵ������ͷ�av_frame_free
AVFrame *XDecode::Recv() 
{
	mux.lock();
	if (!codec) { //���������������
		mux.unlock();
		return NULL;
	}
	AVFrame *frame = av_frame_alloc();
	int re = avcodec_receive_frame(codec,frame);
	mux.unlock();
	if (re !=0) {
		av_frame_free(&frame);
		return NULL;
	}

	cout << "[" << frame->linesize[0] << "] " << flush;
	//av_frame_free(&frame);
	return frame;

}