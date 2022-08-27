#include "XVideoThread.h"
#include "XDecode.h"
//打开,成功与否都要清理资源
bool XVideoThread::Open(AVCodecParameters* para, IVideoCall* call, int width, int herght)
{
	if (!para) return false;

	mux.lock();
	synpts = 0;
	//初始化显示窗口
	this->call = call;
	if (call) {
		call->Init(width,herght);
	}
	//打开解码器
	if (!decode) decode = new XDecode();
	int re = true;
	if (!decode->Open(para)) {
		cout << "audio XDecode open failed !" << endl;
		re = false;
	}
	mux.unlock();
	cout << "XAudioThread open :" << re << endl;
	return re;

};
//多个线程，所以需要一个list队列进行push,然后线程去读AVPacket数据
void XVideoThread::Push(AVPacket* pkt)
{
	if (!pkt) return;
	//阻塞
	while (!isExit)
	{
		mux.lock();
		if (packs.size() < maxList) {
			packs.push_back(pkt);
			mux.unlock();
			break;
		}
		msleep(1);
		mux.unlock();


	}

};
void  XVideoThread::run()
{
	while (!isExit) {
		mux.lock();
		//如果没有数据
		if (packs.empty() || !decode) {
			mux.unlock();
			msleep(1);
			continue;
		}

		//音视频同步，如果快了就等待一下
		if (synpts < decode->pts) {
			mux.unlock();
			msleep(1);
			continue;
		}

		AVPacket* pkt = packs.front();
		packs.pop_front();
		bool re = decode->Send(pkt);
		if (!re) {
			mux.unlock();
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

		mux.unlock();
	}

};

XVideoThread::XVideoThread()
{

};
XVideoThread::~XVideoThread()
{
	//等待线程退出
	isExit = true;
	wait();
};