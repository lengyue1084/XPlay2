#pragma once
#include <QThread>
#include <mutex>
class XDecode;
class XAudioPlay;
class XResample;
struct AVCodecParameters;
//������Ƶ�Ľ��롢�ز��������ŵȣ��൱��mvc��c
class XAudioThread :public QThread
{
public:
	//��,�ɹ����Ҫ������Դ
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

