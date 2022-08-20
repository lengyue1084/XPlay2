#include "XResample.h"

extern "C" {
#include <libswresample/swresample.h>
#include <libavcodec/avcodec.h>

}
#pragma comment(lib,"swresample.lib")

#include <iostream>
using namespace std;

//输出参数喝输入参数一致除了采样格式，输出为S16,会释放para空间
bool XResample::Open(AVCodecParameters* para, bool isClearPara = false)
{
	if (!para) return false;
	mux.lock();
	//音频重采样 上下文初始化
	//SwrContext* actx = swr_alloc();
	actx = swr_alloc_set_opts(actx,
		av_get_default_channel_layout(2),	//输出格式
		//AV_SAMPLE_FMT_S16,				//输出样本格式
		(AVSampleFormat)outFormat,			//输出样本格式
		para->sample_rate,					//输出采样率
		av_get_default_channel_layout(para->channels),//输入格式
		(AVSampleFormat)para->format,
		para->sample_rate,
		0, 0
	);
	avcodec_parameters_free(&para);
	int re = swr_init(actx);

	if (re != 0)
	{
		char buf[1024] = { 0 };
		av_strerror(re, buf, sizeof(buf) - 1);
		cout << "swr_init  failed! :" << buf << endl;
		return false;
	}
	unsigned char* pcm = NULL;
	mux.unlock();
	return true;

};

void XResample::Close()
{
	mux.lock();

	if (actx)
		swr_free(&actx);
	mux.unlock();
};

//返回重采样的大小，不管成功与否都是放indata空间
int XResample::Resample(AVFrame* indata, unsigned char* d)
{
	if (!indata) return 0;

	if (!d) {
		av_frame_free(&indata);
		return 0;
	}
	uint8_t* data[2] = { 0 };
	data[0] = d;
	int re = swr_convert(actx,
		data, indata->nb_samples,		//输出
		(const uint8_t**)indata->data, indata->nb_samples	//输入
	);
	if (re <= 0) return re;
	int outSize = re * indata->channels * av_get_bytes_per_sample((AVSampleFormat)outFormat);
	return outSize;
};
XResample::XResample()
{

};
XResample::~XResample()
{

};
