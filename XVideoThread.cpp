#include "XVideoThread.h"
#include "XDecode.h"
#include <iostream>
using namespace std;
//��,�ɹ����Ҫ������Դ
bool XVideoThread::Open(AVCodecParameters* para, IVideoCall* call, int width, int herght)
{
	if (!para) return false;
	Clear();

	vmux.lock();
	synpts = 0;
	//��ʼ����ʾ����
	this->call = call;
	if (call) {
		call->Init(width,herght);
	}
	vmux.unlock();
	int re = true;
	if (!decode->Open(para)) {
		cout << "audio XDecode open failed !" << endl;
		re = false;
	}

	cout << "XAudioThread open :" << re << endl;
	return re;
}
void XVideoThread::SetPause(bool isPause)
{
	vmux.lock();
	this->isPause = isPause;
	vmux.unlock();
}
void  XVideoThread::run()
{
	while (!isExit) {
		vmux.lock();
		if (this->isPause) {
			vmux.unlock();
			msleep(5);
			continue;
		
		}
		//����Ƶͬ����������˾͵ȴ�һ��
		//������Ҫ����û����Ƶ�����
		if (synpts > 0 && synpts < decode->pts) {
			vmux.unlock();
			msleep(1);
			continue;
		}


		AVPacket *pkt = Pop();
		/*if (!pkt) {
			vmux.unlock();
			continue;
		}*/
		////���û������
		//if (packs.empty() || !decode) {
		//	vmux.unlock();
		//	msleep(1);
		//	continue;
		//}

		//AVPacket* pkt = packs.front();
		//packs.pop_front();
		bool re = decode->Send(pkt);
		if (!re) {
			vmux.unlock();
			msleep(1);
			continue;
		}

		//һ��send,���recv
		while (!isExit)
		{
			AVFrame* frame = decode->Recv();
			if (!frame) break;
			//��ʾ��Ƶ
			if (call) {
				//ˢ�»���
				call->Repaint(frame);
			}

		}

		vmux.unlock();
	}

};
//����pts��������յ��Ľ������� pts >= seekpts return true,������ʾ����
bool XVideoThread::RepaintPts(AVPacket* pkt, long long seekpts)
{
	vmux.lock();
	bool re = decode->Send(pkt);
	if (!re) {//��ʾ��������
		vmux.unlock();
		return false;
	}
	AVFrame* frame = decode->Recv();
	if (!frame) {//false������һ�δ���
		vmux.unlock();
		return false;
	}
	//����λ��
	if (decode->pts >= seekpts) {
		if (call) {
			call->Repaint(frame);
			vmux.unlock();
			return true;//�������һ֡
		}
	}
	XFreeFrame(&frame);
	vmux.unlock();

	return false;
}


XVideoThread::XVideoThread()
{

};
XVideoThread::~XVideoThread()
{

};