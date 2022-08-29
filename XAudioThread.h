#pragma once
#include <QThread>
#include <mutex>
#include <list>
struct AVCodecParameters;
class XAudioPlay;
class XResample;
#include "XDecodeThread.h"
//������Ƶ�Ľ��롢�ز��������ŵȣ��൱��mvc��c
class XAudioThread :public XDecodeThread
{
public:

	//��ǰ������Ƶ��pts
	long long pts = 0;

	//��,�ɹ����Ҫ������Դ
	virtual bool Open(AVCodecParameters* para, int sampleRate, int channels);
	//����̣߳�������Ҫһ��list���н���push,Ȼ���߳�ȥ��AVPacket����
	//virtual void Push(AVPacket *pkt);
	void run();
	XAudioThread();
	virtual ~XAudioThread();

	////������
	//int maxList = 100*100;
	//bool isExit = false;

protected:
	//std::list<AVPacket*> packs;
	std::mutex amux;
	XDecode* decode = 0;
	XAudioPlay* ap = 0;
	XResample* res = 0;
};

