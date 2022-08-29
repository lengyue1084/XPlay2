#include "XDecodeThread.h"
#include "XDecode.h"
#include <iostream>
#include <list>
#include <mutex>
#include <QThread>
using namespace std;
struct XDecode;

//清理队列
void XDecodeThread::Clear()
{
	mux.lock();
	decode->Clear();
	while (!packs.empty())
	{
		AVPacket* pkt = packs.front();
		XFreePacket(&pkt);
		packs.pop_front();
	}
	mux.unlock();
}
void XDecodeThread::Close()
{

}
//取出一帧数据，并出栈，如果没有返回NULL
AVPacket* XDecodeThread::Pop()
{

	mux.lock();
	if (packs.empty()) {
		mux.unlock();
		return NULL;
	}
	AVPacket* pkt = packs.front();
	packs.pop_front();
	mux.unlock();

	return pkt;
};
//多个线程，所以需要一个list队列进行push,然后线程去读AVPacket数据
void XDecodeThread::Push(AVPacket* pkt)
{
	if (!pkt) return;
	//阻塞
	while (!isExit)
	{
		mux.lock();
		if (packs.size() < maxList) {
			packs.push_back(pkt);
			mux.unlock();
			break;
		}
		msleep(1);
		mux.unlock();


	}

};
XDecodeThread::XDecodeThread() 
{
	//打开解码器
	if (!decode) decode = new XDecode();

}
 XDecodeThread::~XDecodeThread()
{
	 //等待线程退出
	 isExit = true;
	 wait();
}