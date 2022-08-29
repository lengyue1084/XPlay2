#include "XVideoThread.h"
#include "XDecode.h"
#include <iostream>
using namespace std;
//打开,成功与否都要清理资源
bool XVideoThread::Open(AVCodecParameters* para, IVideoCall* call, int width, int herght)
{
	if (!para) return false;
	Clear();

	vmux.lock();
	synpts = 0;
	//初始化显示窗口
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

void  XVideoThread::run()
{
	while (!isExit) {
		vmux.lock();
		//音视频同步，如果快了就等待一下
		//这里需要考虑没有音频的情况
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
		////如果没有数据
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

		//一次send,多次recv
		while (!isExit)
		{
			AVFrame* frame = decode->Recv();
			if (!frame) break;
			//显示视频
			if (call) {
				call->Repaint(frame);
			}

		}

		vmux.unlock();
	}

};

XVideoThread::XVideoThread()
{

};
XVideoThread::~XVideoThread()
{

};