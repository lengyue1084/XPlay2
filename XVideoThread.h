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
	//��,�ɹ����Ҫ������Դ
	virtual bool Open(AVCodecParameters* para, IVideoCall* call, int width, int herght);

	void run();


	XVideoThread();
	virtual ~XVideoThread();


	//ͬ��ʱ�䣬���ⲿ����
	long long synpts = 0;

protected:
	std::mutex vmux;
	IVideoCall* call = 0;
};

