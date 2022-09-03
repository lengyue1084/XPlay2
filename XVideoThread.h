#pragma once

struct AVPacket;
struct AVCodecParameters;
class XDecode;
#include <list>
#include <mutex>
#include <QThread>
#include "IVideoCall.h"
#include "XDecodeThread.h"
using namespace std;
class XVideoThread : public XDecodeThread
{
public:
	//����pts��������յ��Ľ������� pts >= seekpts return true,������ʾ����
	virtual bool RepaintPts(AVPacket *pkt,long long seekpts);
	//��,�ɹ����Ҫ������Դ
	virtual bool Open(AVCodecParameters* para, IVideoCall* call, int width, int herght);
	void run();
	XVideoThread();
	virtual ~XVideoThread();

	//ͬ��ʱ�䣬���ⲿ����
	long long synpts = 0;

	bool isPause = false;
	void SetPause(bool isPause);


protected:
	IVideoCall* call = 0;
	std::mutex vmux;

};

