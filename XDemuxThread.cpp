#include "XDemuxThread.h"
#include "XDemux.h"
#include "XVideoThread.h"
#include "XAudioThread.h"
#include <iostream>
using namespace std;
//一般头文件中不引入.h文件，再cpp中引入
bool XDemuxThread::Open(const char* url, IVideoCall* call)
{
	if (url == 0 || url[0] == '\0') return false;
	mux.lock();
	if (!demux) demux = new XDemux();
	if (!vt) vt = new XVideoThread();
	if (!at) at = new XAudioThread();

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

	if (!at ->Open(demux->CopyAPara(), demux->sampleRate, demux->channels)) {
		re = false;
		cout << "at->Open failed!" << endl;
	
	}
	cout << "XDemuxThread::Open!" << endl;
	mux.unlock();
	return true;
};
XDemuxThread::XDemuxThread()
{
};
XDemuxThread::~XDemuxThread()
{
};