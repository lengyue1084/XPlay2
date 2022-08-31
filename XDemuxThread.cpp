#include "XDemuxThread.h"
#include "XDemux.h"
#include "XVideoThread.h"
#include "XAudioThread.h"
#include <iostream>
using namespace std;

//��ͣ���vt/at��ͣ������
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

		//����Ƶͬ����û�п���ֻ����Ƶ������Ƶ�����
		if (vt && at)
		{
			pts = at->pts;
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
	//cout << "XDemuxThread::Open!" << endl;

	totalMs = demux->totalMs;//��ʱ��
	//this->totalMs = demux->totalMs;//��ʱ��
	mux.unlock();
	return true;
};

//�ر��߳�������Դ
void XDemuxThread::Close()
{
	isExit = true;
	//qt����ģ��ȴ��߳��˳�
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