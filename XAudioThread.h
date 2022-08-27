#pragma once
#include <QThread>
#include <mutex>
#include <list>

class XDecode;
class XAudioPlay;
class XResample;
class AVPacket;
struct AVCodecParameters;
//管理音频的解码、重采样、播放等，相当于mvc的c
class XAudioThread :public QThread
{
public:

	//当前播放音频的pts
	long long pts = 0;

	//打开,成功与否都要清理资源
	virtual bool Open(AVCodecParameters* para,int sampleRate,int channels);
	//多个线程，所以需要一个list队列进行push,然后线程去读AVPacket数据
	virtual void Push(AVPacket *pkt);
	void run();
	XAudioThread();
	virtual ~XAudioThread();

	//最大队列
	int maxList = 100*100;
	bool isExit = false;

protected:
	std::list<AVPacket*> packs;
	std::mutex mux;
	XDecode* decode = 0;
	XAudioPlay* ap = 0;
	XResample *res = 0;
};

