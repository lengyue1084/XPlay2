#include "XAudioThread.h"
#include "XDecode.h"
#include "XAudioPlay.h"
#include "XResample.h"
#include <iostream>

extern "C" {
#include <libswresample/swresample.h>
#include <libavcodec/avcodec.h>

}
using namespace std;
//ֹͣ�̡߳��ر������Դ
void XAudioThread::Close()
{
	//�ȵ��ø����Close����
	XDecodeThread::Close();
	if (res)
	{
		res->Close();
		amux.lock();
		delete res;
		res = NULL;
		amux.unlock();
	}

	if (ap)
	{
		ap->Close();
		amux.lock();
		ap = NULL;//����Ҫdelete
		amux.unlock();
	}
}
bool XAudioThread::Open(AVCodecParameters* para, int sampleRate, int channels)
{
	if (!para) return false;
	Clear();
	amux.lock();
	//������һ�ε�Ӱ��
	pts = 0;
	/*if (!decode) decode = new XDecode();
	if (!res) res = new XResample();
	if (!ap) ap = XAudioPlay::Get();*/
	bool re = true;
	if (!res->Open(para,false)) {
		//mux.unlock();
		//return false;
		cout << "XResample open failed !" << endl;
		re = false;
	}

	ap->sampleRate = sampleRate;
	ap->channels = channels;
	if (!ap->Open()) {
		cout << "XAudioPlay open failed !" << endl;
		//mux.unlock();
		//return false;
		re = false;
	}
	if (!decode->Open(para)) {
		cout << "audio XDecode open failed !" << endl;
		//mux.unlock();
		//return false;
		re = false;
	}
	amux.unlock();
	//cout << "XAudioThread open :" << re << endl;
	return re;

};
//��Ƶ����Ҫ����ͬ�����⣬���õ�����Ƶͬ����Ƶ�ķ���
void XAudioThread::run()
{
	unsigned char* pcm = new unsigned char[1024 * 1024 * 10];
	while (!isExit) {
		amux.lock();
		////���û������
		//if (packs.empty() || !decode || !res || !ap) {
		//	amux.unlock();
		//	msleep(1);
		//	continue;
		//}
	
		//AVPacket *pkt = packs.front();
		//packs.pop_front();
		AVPacket* pkt = Pop();
		/*if (!pkt) {
			amux.unlock();
			msleep(5);
			continue;
		}*/
		bool re = decode->Send(pkt);
		if (!re) {
			amux.unlock();
			msleep(1);
			continue;
		}

		//һ��send,���recv
		while (!isExit)
		{
			AVFrame* frame = decode->Recv();
			if (!frame) break;
			//��ȥ������δ���ŵ�ʱ��
			pts = decode->pts - ap->GetNoPlayMs();
			//cout << "audio pts = " << pts << endl;
			//�ز���
			int size = res->Resample(frame,pcm);//���ͷ�frame�ռ䣬�˴�����Ҫ�ͷ�

			av_frame_free(&frame);
			//������Ƶ
			while (!isExit)
			{
				if (size <= 0) break;
				//����δ�����꣬�ռ䲻��
				if (ap->GetFree() < size) {
					msleep(1);
					continue;
				}
			
				ap->Write(pcm,size);
				break;

			}
		}

		amux.unlock();
	}
	delete []pcm;
	//delete pcm;

};
XAudioThread::XAudioThread()
{

	if (!res) res = new XResample();
	if (!ap) ap = XAudioPlay::Get();

};
XAudioThread::~XAudioThread()
{
	//�ȴ��߳��˳�
	isExit = true;
	wait();

};