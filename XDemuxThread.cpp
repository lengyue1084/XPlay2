#include "XDemuxThread.h"
#include "XDemux.h"
#include "XVideoThread.h"
#include "XAudioThread.h"
#include <iostream>
extern "C" {

#include <libavformat/avformat.h>
}
#include "XDecode.h"
using namespace std;
void XDemuxThread::Clear()
{
	mux.lock();
	if (demux) demux->Clear();
	if (vt) vt->Clear();
	if (at) at->Clear();

	mux.unlock();
}
void XDemuxThread::Seek(double pos)
{
	//������
	Clear();
	mux.lock();
	bool status = this->isPause;
	mux.unlock();
	//��ͣ
	SetPause(true);

	mux.lock();
	if (demux)
		demux->Seek(pos);
	//ʵ��Ҫ��ʾ��λ��pts

	long long seekPts = pos * demux->totalMs;
	while (!isExit)
	{
		AVPacket* pkt = demux->Read();
		if (!pkt) break;
		if (pkt->stream_index == demux->audioStream) {

			//����Ƶ����,����
			av_packet_free(&pkt);
			continue;
		}
		bool re = vt->decode->Send(pkt);
		if (!re) break;
		AVFrame* frame = vt->decode->Recv();
		if (!frame) continue;
		//����λ��
		if (frame->pts >= seekPts) {

			this->pts = frame->pts;
			vt->call->Repaint(frame);
			break;
		}
		av_frame_free(&frame);

	}
	mux.unlock();
	//seek�Ƿ���ͣ״̬���ָ�����
	if (!status)
		SetPause(false);
}
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