#pragma once
#include <QThread>
#include <mutex>
class XDecode;
class XAudioPlay;
class XResample;
struct AVCodecParameters;
//管理音频的解码、重采样、播放等，相当于mvc的c
class XAudioThread :public QThread
{
public:
	//打开,成功与否都要清理资源
	virtual bool Open(AVCodecParameters* para);
	void Run();
	XAudioThread();
	virtual ~XAudioThread();

protected:
	std::mutex mux;
	XDecode* decode = 0;
	XAudioPlay* ap = 0;
	XResample* res = 0;
};

