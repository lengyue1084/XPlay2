#include "XDemuxThread.h"
#include "XDemux.h"
#include "XVideoThread.h"
#include "XAudioThread.h"
#include <iostream>
using namespace std;

//暂停设计vt/at暂停的问题
void XDemuxThread::SetPause(bool isPause)
{
	mux.lock();
	this->isPause = isPause;
	if (at) at->SetPause(isPause);
	if (vt) vt->SetPause(isPause);
	mux.unlock();
}
void XDemuxThread::run()
{

	while (!isExit)
	{
		mux.lock();
		if (isPause) {
			mux.unlock();
			msleep(5);
			continue;
		}
		if (!demux) {
			mux.unlock();
			msleep(5);
			continue;
		}

		//音视频同步，没有考虑只有音频或者视频的情况
		if (vt && at)
		{
			pts = at->pts;
			vt->synpts = at->pts;
		}
		AVPacket* pkt = demux->Read();
		//没有读取到视频帧
		if (!pkt) {
			mux.unlock();
			msleep(5);
			continue;
		}
		//读取到数据判断数据类型
		if (demux->IsAudio(pkt)) {

			if (at) at->Push(pkt);
		}
		else {//视频
			if (vt)vt->Push(pkt);
		}

		mux.unlock();
		msleep(1);
	}

}

//一般头文件中不引入.h文件，再cpp中引入
bool XDemuxThread::Open(const char* url, IVideoCall* call)
{
	if (url == 0 || url[0] == '\0') return false;
	mux.lock();

	//打开解封装
	bool re = demux->Open(url);
	if (!re) {
		mux.unlock();
		cout << "demux->Open(url) failed!" << endl;
		return false;
	}
	//打开视频解码器和处理线程
	if (!vt->Open(demux->CopyVPara(), call, demux->width, demux->height)) {
		re = false;
		cout << "vt->Open failed!" << endl;
	};

	if (!at->Open(demux->CopyAPara(), demux->sampleRate, demux->channels)) {
		re = false;
		cout << "at->Open failed!" << endl;

	}
	//cout << "XDemuxThread::Open!" << endl;

	totalMs = demux->totalMs;//总时长
	//this->totalMs = demux->totalMs;//总时长
	mux.unlock();
	return true;
};

//关闭线程清理资源
void XDemuxThread::Close()
{
	isExit = true;
	//qt里面的，等待线程退出
	wait();
	if (vt) vt->Close();
	if (at) at->Close();
	mux.lock();
	delete vt;
	delete at;
	vt = NULL;
	at = NULL;
	mux.unlock();

}

//启动所有线程
void XDemuxThread::Start()
{
	mux.lock();
	if (!demux) demux = new XDemux();
	if (!vt) vt = new XVideoThread();
	if (!at) at = new XAudioThread();
	QThread::start();
	if (vt) vt->start();
	if (at) at->start();
	mux.unlock();

};


XDemuxThread::XDemuxThread()
{
};
XDemuxThread::~XDemuxThread()
{
	isExit = true;
	wait();
};