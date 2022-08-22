#pragma once
#include <mutex>
struct AVCodecParameters;
struct AVFrame;
struct SwrContext;
class XResample
{
public:
	//����������������һ�³��˲�����ʽ�����ΪS16,���ͷ�para�ռ�
	virtual bool Open(AVCodecParameters *para,bool isClearPara = false);
	virtual void Close();

	//�����ز����Ĵ�С�����ܳɹ�����Ƿ�indata�ռ�
	virtual int Resample(AVFrame *indata,unsigned char *data);
	XResample();
	~XResample();
	//AV_SAMPLE_FMT_S16
	int outFormat = 1;
protected:
	std::mutex mux;
	SwrContext *actx = 0;
};

