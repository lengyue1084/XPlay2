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
	 if (codec) {//关闭的时候清理解码缓存了
		 avcodec_close(codec);
		 avcodec_free_context(&codec);
	 }
	 mux.unlock();
}

 void XDecode::Clear() {
	 mux.lock();
	 //清理解码缓冲
	 if (codec) avcodec_flush_buffers(codec);
	 mux.unlock();

}

bool XDecode::Open(AVCodecParameters *para)
{
	//Close();
	if (!para) return false;
	Close();
	//////////////////////////////////////////////////////////
	///视频解码器打开
	///找到视频解码器
	AVCodec *vcodec = avcodec_find_decoder(para->codec_id);
	if (!vcodec)
	{
		avcodec_parameters_free(&para);
		cout << "can't find the codec id " << para->codec_id << endl;
		return false;
	}
	cout << "find the AVCodec " << para->codec_id << endl;

	mux.lock();//此处开始访问共享变量
	//AVCodecContext *codec = avcodec_alloc_context3(vcodec);
	 codec = avcodec_alloc_context3(vcodec);

	///配置解码器上下文参数
	int re = avcodec_parameters_to_context(codec, para);
	if (re < 0) {
		cout << "avcodec_parameters_to_context  failed! :" << endl;
		mux.unlock();
		return false;
	}
	//八线程解码
	codec->thread_count = 8;

	///打开解码器上下文
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

//发送到解码线程，不管成功与否清理pkt空间，对象和媒体内容空间
bool XDecode::Send(AVPacket *pkt) {

	// 盘错
	if (!pkt || pkt->size <= 0 || !pkt->data) return false;
	mux.lock();
	if (!codec) { //如果加码器不存在
		mux.unlock();
		return false;
	}
	int re = avcodec_send_packet(codec,pkt);
	mux.unlock();
	av_packet_free(&pkt);//与分配是成对的
	if (re != 0) return false;
	return true;
}


//获取解码数据，一次send可能需要多次Recv,获取缓冲中的数据Send NULL再Recv中多次
//每次赋值一份，由调用者释放av_frame_free
AVFrame *XDecode::Recv() 
{
	mux.lock();
	if (!codec) { //如果解码器不存在
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