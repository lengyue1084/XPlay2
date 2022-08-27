#include "XDemux.h"
#include <iostream>
using namespace std;
extern "C" {
#include "libavformat/avformat.h"
}
#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avutil.lib")
#pragma comment(lib,"avcodec.lib")
static double r2d(AVRational r)
{
	return r.den == 0 ? 0 : (double)r.num / (double)r.den;
}

bool XDemux::Open(const char *url) 
{
	Close();
	//参数设置
	AVDictionary* opts = NULL;
	//设置rtsp流已tcp协议打开
	av_dict_set(&opts, "rtsp_transport", "tcp", 0);

	//网络延时时间
	av_dict_set(&opts, "max_delay", "500", 0);


	//解封装上下文
	//AVFormatContext* ic = NULL;
	mux.lock();
	int re = avformat_open_input(
		&ic,
		url,
		0,  // 0表示自动选择解封器
		&opts //参数设置，比如rtsp的延时时间
	);
	if (re != 0)
	{
		mux.unlock();
		char buf[1024] = { 0 };
		cout << "open " << url << " failed! :" << buf << endl;
		//getchar();
		return false;
	}
	cout << "open " << url << " success! " << endl;

	//获取流信息 
	re = avformat_find_stream_info(ic, 0);

	//总时长 毫秒
	totalMs = ic->duration / (AV_TIME_BASE / 1000);
	cout << "totalMs = " << totalMs << endl;

	//打印视频流详细信息
	av_dump_format(ic, 0, url, 0);

	//获取音视频流信息 （遍历，函数获取）
	//获取视频流
	videoStream = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	AVStream* as = ic->streams[videoStream];
	width = as->codecpar->width;
	height = as->codecpar->height;
	cout << "=============================================" << endl;
	cout << "codec_id = " << as->codecpar->codec_id << endl;
	cout << "format = " << as->codecpar->format << endl;
	cout << videoStream << "视频信息" << endl;
	cout << "width=" << as->codecpar->width << endl;
	cout << "height=" << as->codecpar->height << endl;

	//帧率 fps 分数转换
	cout << "video fps = " << r2d(as->avg_frame_rate) << endl;
	cout << "=============================================" << endl;
	//获取音频流
	audioStream = av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
	as = ic->streams[audioStream];
	sampleRate = as->codecpar->sample_rate;//样本率
	channels = as->codecpar->channels;
	cout << "codec_id = " << as->codecpar->codec_id << endl;
	cout << "format = " << as->codecpar->format << endl;
	cout << "sample_rate = " << as->codecpar->sample_rate << endl;
	//AVSampleFormat;
	cout << "channels = " << as->codecpar->channels << endl;
	//一帧数据？？ 单通道样本数 
	cout << "frame_size = " << as->codecpar->frame_size << endl;
	//1024 * 2 * 2 = 4096  fps = sample_rate/frame_size

	mux.unlock();
	return true;

}
//清空读取缓存
void XDemux::Clear()
{
	mux.lock();
	if (!ic) {
		mux.unlock();
		return;
	}
	int re = avformat_flush(ic);
	mux.unlock();
	if (re < 0) {
		return;
	}
}

void  XDemux::Close() {
	mux.lock();
	if (!ic) {
		mux.unlock();
		return;
	}
	avformat_close_input(&ic);//这里会清空所有缓存
	totalMs = 0;
	mux.unlock();

}

//seek 位置 0.0~1.0
bool XDemux::Seek(double pos)
{
	mux.lock();
	if (!ic) {
		mux.unlock();
		return false;
	}
	long long seekPos = 0;
	//考虑ic->streams[videoStream]->duration 不存在的情况
	//if(!ic->streams[videoStream]->duration) {
	//	//pos = (double)ms / (double)1000 * r2d(ic->streams[pkt->stream_index]->time_base);
	//}
	//清理读取缓冲，读取到了新的位置，防止网络粘包现象
	int re = avformat_flush(ic);
	if (re < 0) {
		mux.unlock();
		return false;
	}
	seekPos = ic->streams[videoStream]->duration * pos;

	//int ms = 3000; //三秒位置 根据时间基数（分数）转换
	//long long pos = (double)ms / (double)1000 * r2d(ic->streams[pkt->stream_index]->time_base);
	//seek只是往后跳到关键帧，实际帧还是需要业务来做的，这里需要跟解码模块关联
	re = av_seek_frame(ic, videoStream, seekPos, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
	mux.unlock();
	return true;
};

//获取视频参数,返回的空间需要清理 avcodec_parameters_free
AVCodecParameters *XDemux::CopyVPara() 
{
	mux.lock();
	if (!ic) {
		mux.unlock();
		return NULL;
	}
	AVCodecParameters *pa = avcodec_parameters_alloc();
	int re = avcodec_parameters_copy(pa,ic->streams[videoStream]->codecpar);
	if (re != 0) {
		mux.unlock();
		return NULL;
	}
	mux.unlock();
	return pa;
};

//获取音频参数,返回的空间需要清理 avcodec_parameters_free
AVCodecParameters *XDemux::CopyAPara()
{
	mux.lock();
	if (!ic) {
		mux.unlock();
		return NULL;
	}
	AVCodecParameters* pa = avcodec_parameters_alloc();
	int re = avcodec_parameters_copy(pa, ic->streams[audioStream]->codecpar);
	if (re != 0) {
		mux.unlock();
		return NULL;
	}
	mux.unlock();
	return pa;
}

//判断音视频
bool XDemux::IsAudio(AVPacket *pkt)
{
	if (!pkt) return false;
	if (pkt->stream_index == videoStream) 
		return false;

	return true;

};


//空间需要调用者释放，释放AVPacket对象空间，和数据空间 av_packet_free
AVPacket *XDemux::Read()
{
	mux.lock();
	if (!ic) {
		mux.unlock();
		return false;
	}
	//分配对象空间
	AVPacket *pkt = av_packet_alloc();
	//读取一帧，并分配空间
	int re = av_read_frame(ic,pkt);
	if (re != 0) {
		mux.unlock();
		av_packet_free(&pkt);
		return false;
	}
	//pts转换成毫秒
	pkt->pts = pkt->pts * (1000 * (r2d(ic->streams[pkt->stream_index]->time_base)));
	pkt->dts = pkt->dts * (1000 * (r2d(ic->streams[pkt->stream_index]->time_base)));
	mux.unlock();
	//cout << "pkt->pts:" <<  pkt->pts << " " << flush;

	return pkt;

}

XDemux::XDemux() 
{
	static bool isFirst = true;
	static std::mutex dmux;
	dmux.lock();
	if (isFirst) {
		// 初始化封装库
		av_register_all();

		//初始化网络库（可以打开rtsp rtmp http 协议的流媒体视频）
		avformat_network_init();
		isFirst = false;
	}
	dmux.unlock();

}
XDemux::~XDemux() 
{

}
