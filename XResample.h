#pragma once
#include <mutex>
struct AVCodecParameters;
struct AVFrame;
struct SwrContext;
class XResample
{
public:
	//输出参数和输入参数一致除了采样格式，输出为S16,会释放para空间
	virtual bool Open(AVCodecParameters *para,bool isClearPara = false);
	virtual void Close();

	//返回重采样的大小，不管成功与否都是放indata空间
	virtual int Resample(AVFrame *indata,unsigned char *data);
	XResample();
	~XResample();
	//AV_SAMPLE_FMT_S16
	int outFormat = 1;
protected:
	std::mutex mux;
	SwrContext *actx = 0;
};

