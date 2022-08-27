#include "XDemuxThread.h"
#include "XDemux.h"
#include "XVideoThread.h"
#include "XAudioThread.h"
#include <iostream>
using namespace std;

void XDemuxThread::run()
{

	while (!isExit)
	{
		mux.lock();
		if (!demux) {
			mux.unlock();
			msleep(5);
			continue;
		}

		//����Ƶͬ����û�п���ֻ����Ƶ������Ƶ�����
		if (vt && at)
		{
			vt->synpts = at->pts;
		}
		AVPacket* pkt = demux->Read();
		//û�ж�ȡ����Ƶ֡
		if (!pkt) {
			mux.unlock();
			msleep(5);
			continue;
		}
		//��ȡ�������ж���������
		if (demux->IsAudio(pkt)) {

			if (at) at->Push(pkt);
		}
		else {//��Ƶ
			if (vt)vt->Push(pkt);
		}

		mux.unlock();
		msleep(1);
	}

}

//һ��ͷ�ļ��в�����.h�ļ�����cpp������
bool XDemuxThread::Open(const char* url, IVideoCall* call)
{
	if (url == 0 || url[0] == '\0') return false;
	mux.lock();

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

	if (!at->Open(demux->CopyAPara(), demux->sampleRate, demux->channels)) {
		re = false;
		cout << "at->Open failed!" << endl;

	}
	cout << "XDemuxThread::Open!" << endl;
	mux.unlock();
	return true;
};

//���������߳�
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