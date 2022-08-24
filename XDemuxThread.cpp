#include "XDemuxThread.h"
#include "XDemux.h"
#include "XVideoThread.h"
#include "XAudioThread.h"
#include <iostream>
using namespace std;
//һ��ͷ�ļ��в�����.h�ļ�����cpp������
bool XDemuxThread::Open(const char* url, IVideoCall* call)
{
	if (url == 0 || url[0] == '\0') return false;
	mux.lock();
	if (!demux) demux = new XDemux();
	if (!vt) vt = new XVideoThread();
	if (!at) at = new XAudioThread();

	//�򿪽��װ
	bool re = demux->Open(url);
	if (!re) {
		mux.unlock();
		cout << "demux->Open(url) failed!" << endl;
		return false;
	}
	//����Ƶ�������ʹ����߳�
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