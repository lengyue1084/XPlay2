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
	//解码pts，如果接收到的解码数据 pts >= seekpts return true,并且显示画面
	virtual bool RepaintPts(AVPacket *pkt,long long seekpts);
	//打开,成功与否都要清理资源
	virtual bool Open(AVCodecParameters* para, IVideoCall* call, int width, int herght);
	void run();
	XVideoThread();
	virtual ~XVideoThread();

	//同步时间，由外部传入
	long long synpts = 0;

	bool isPause = false;
	void SetPause(bool isPause);


protected:
	IVideoCall* call = 0;
	std::mutex vmux;

};

