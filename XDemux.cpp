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
	//��������
	AVDictionary* opts = NULL;
	//����rtsp����tcpЭ���
	av_dict_set(&opts, "rtsp_transport", "tcp", 0);

	//������ʱʱ��
	av_dict_set(&opts, "max_delay", "500", 0);


	//���װ������
	//AVFormatContext* ic = NULL;
	mux.lock();
	int re = avformat_open_input(
		&ic,
		url,
		0,  // 0��ʾ�Զ�ѡ������
		&opts //�������ã�����rtsp����ʱʱ��
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

	//��ȡ����Ϣ 
	re = avformat_find_stream_info(ic, 0);

	//��ʱ�� ����
	totalMs = ic->duration / (AV_TIME_BASE / 1000);
	cout << "totalMs = " << totalMs << endl;

	//��ӡ��Ƶ����ϸ��Ϣ
	av_dump_format(ic, 0, url, 0);

	//��ȡ����Ƶ����Ϣ ��������������ȡ��
	//��ȡ��Ƶ��
	videoStream = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	AVStream* as = ic->streams[videoStream];
	width = as->codecpar->width;
	height = as->codecpar->height;
	cout << "=============================================" << endl;
	cout << "codec_id = " << as->codecpar->codec_id << endl;
	cout << "format = " << as->codecpar->format << endl;
	cout << videoStream << "��Ƶ��Ϣ" << endl;
	cout << "width=" << as->codecpar->width << endl;
	cout << "height=" << as->codecpar->height << endl;

	//֡�� fps ����ת��
	cout << "video fps = " << r2d(as->avg_frame_rate) << endl;
	cout << "=============================================" << endl;
	//��ȡ��Ƶ��
	audioStream = av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
	as = ic->streams[audioStream];
	sampleRate = as->codecpar->sample_rate;//������
	channels = as->codecpar->channels;
	cout << "codec_id = " << as->codecpar->codec_id << endl;
	cout << "format = " << as->codecpar->format << endl;
	cout << "sample_rate = " << as->codecpar->sample_rate << endl;
	//AVSampleFormat;
	cout << "channels = " << as->codecpar->channels << endl;
	//һ֡���ݣ��� ��ͨ�������� 
	cout << "frame_size = " << as->codecpar->frame_size << endl;
	//1024 * 2 * 2 = 4096  fps = sample_rate/frame_size

	mux.unlock();
	return true;

}
//��ն�ȡ����
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
	avformat_close_input(&ic);//�����������л���
	totalMs = 0;
	mux.unlock();

}

//seek λ�� 0.0~1.0
bool XDemux::Seek(double pos)
{
	mux.lock();
	if (!ic) {
		mux.unlock();
		return false;
	}
	long long seekPos = 0;
	//����ic->streams[videoStream]->duration �����ڵ����
	//if(!ic->streams[videoStream]->duration) {
	//	//pos = (double)ms / (double)1000 * r2d(ic->streams[pkt->stream_index]->time_base);
	//}
	//�����ȡ���壬��ȡ�����µ�λ�ã���ֹ����ճ������
	int re = avformat_flush(ic);
	if (re < 0) {
		mux.unlock();
		return false;
	}
	seekPos = ic->streams[videoStream]->duration * pos;

	//int ms = 3000; //����λ�� ����ʱ�������������ת��
	//long long pos = (double)ms / (double)1000 * r2d(ic->streams[pkt->stream_index]->time_base);
	//seekֻ�����������ؼ�֡��ʵ��֡������Ҫҵ�������ģ�������Ҫ������ģ�����
	re = av_seek_frame(ic, videoStream, seekPos, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
	mux.unlock();
	return true;
};

//��ȡ��Ƶ����,���صĿռ���Ҫ���� avcodec_parameters_free
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

//��ȡ��Ƶ����,���صĿռ���Ҫ���� avcodec_parameters_free
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

//�ж�����Ƶ
bool XDemux::IsAudio(AVPacket *pkt)
{
	if (!pkt) return false;
	if (pkt->stream_index == videoStream) 
		return false;

	return true;

};


//�ռ���Ҫ�������ͷţ��ͷ�AVPacket����ռ䣬�����ݿռ� av_packet_free
AVPacket *XDemux::Read()
{
	mux.lock();
	if (!ic) {
		mux.unlock();
		return false;
	}
	//�������ռ�
	AVPacket *pkt = av_packet_alloc();
	//��ȡһ֡��������ռ�
	int re = av_read_frame(ic,pkt);
	if (re != 0) {
		mux.unlock();
		av_packet_free(&pkt);
		return false;
	}
	//ptsת���ɺ���
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
		// ��ʼ����װ��
		av_register_all();

		//��ʼ������⣨���Դ�rtsp rtmp http Э�����ý����Ƶ��
		avformat_network_init();
		isFirst = false;
	}
	dmux.unlock();

}
XDemux::~XDemux() 
{

}
