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
				//刷新画面
				call->Repaint(frame);
			}

		}

		vmux.unlock();
	}

};
//解码pts，如果接收到的解码数据 pts >= seekpts return true,并且显示画面
bool XVideoThread::RepaintPts(AVPacket* pkt, long long seekpts)
{
	vmux.lock();
	bool re = decode->Send(pkt);
	if (!re) {//表示结束解码
		vmux.unlock();
		return false;
	}
	AVFrame* frame = decode->Recv();
	if (!frame) {//false继续下一次处理
		vmux.unlock();
		return false;
	}
	//到达位置
	if (decode->pts >= seekpts) {
		if (call) {
			call->Repaint(frame);
			vmux.unlock();
			return true;//到了最后一帧
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