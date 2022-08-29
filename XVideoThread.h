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
	//打开,成功与否都要清理资源
	virtual bool Open(AVCodecParameters* para, IVideoCall* call, int width, int herght);

	void run();


	XVideoThread();
	virtual ~XVideoThread();


	//同步时间，由外部传入
	long long synpts = 0;

protected:
	std::mutex vmux;
	IVideoCall* call = 0;
};

