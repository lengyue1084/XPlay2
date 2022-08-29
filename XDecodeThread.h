#pragma once

struct AVPacket;
class XDecode;
#include <list>
#include <QThread>
#include "XDecode.h"
class XDecodeThread:public QThread
{
public:
	XDecodeThread();
	virtual ~XDecodeThread();
	//多个线程，所以需要一个list队列进行push,然后线程去读AVPacket数据
	virtual void Push(AVPacket* pkt);

	//清理队列
	virtual void Clear();
	virtual void Close();

	//取出一帧数据，并出栈，如果没有返回NULL
	virtual AVPacket *Pop();

	//最大队列
	//int maxList = 100 * 100;
	int maxList = 100;
	bool isExit = false;
protected:
	std::list<AVPacket *> packs;
	std::mutex mux;
	XDecode* decode = 0;
};

