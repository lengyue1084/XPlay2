#pragma once
#include <iostream>
#include <list>
#include <mutex>
#include <QThread>
#include "IVideoCall.h"
using namespace std;
struct AVPacket;
struct XDecode;
struct AVCodecParameters;
class XVideoThread : public QThread
{
public:
	//打开,成功与否都要清理资源
	virtual bool Open(AVCodecParameters* para, IVideoCall* call, int width, int herght);
	//多个线程，所以需要一个list队列进行push,然后线程去读AVPacket数据
	virtual void Push(AVPacket* pkt);
	void run();


	XVideoThread();
	virtual ~XVideoThread();

	//最大队列
	int maxList = 100 * 100;
	bool isExit = false;

protected:
	std::list<AVPacket*> packs;
	std::mutex mux;
	XDecode* decode = 0;
	IVideoCall* call = 0;
};

