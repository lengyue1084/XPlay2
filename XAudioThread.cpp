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
//停止线程、关闭相关资源
void XAudioThread::Close()
{
	//先调用父类的Close方法
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
		ap = NULL;//不需要delete
		amux.unlock();
	}
}
bool XAudioThread::Open(AVCodecParameters* para, int sampleRate, int channels)
{
	if (!para) return false;
	Clear();
	amux.lock();
	//不受上一次的影响
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
//音频不需要考虑同步问题，采用的是视频同步音频的方案
void XAudioThread::run()
{
	unsigned char* pcm = new unsigned char[1024 * 1024 * 10];
	while (!isExit) {
		amux.lock();
		////如果没有数据
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

		//一次send,多次recv
		while (!isExit)
		{
			AVFrame* frame = decode->Recv();
			if (!frame) break;
			//减去缓冲中未播放的时间
			pts = decode->pts - ap->GetNoPlayMs();
			//cout << "audio pts = " << pts << endl;
			//重采样
			int size = res->Resample(frame,pcm);//会释放frame空间，此处不需要释放

			av_frame_free(&frame);
			//播放音频
			while (!isExit)
			{
				if (size <= 0) break;
				//缓冲未播放完，空间不够
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
	//等待线程退出
	isExit = true;
	wait();

};