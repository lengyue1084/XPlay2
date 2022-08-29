#include "XDecodeThread.h"
#include "XDecode.h"
#include <iostream>
#include <list>
#include <mutex>
#include <QThread>
using namespace std;
struct XDecode;

//�������
void XDecodeThread::Clear()
{
	mux.lock();
	decode->Clear();
	while (!packs.empty())
	{
		AVPacket* pkt = packs.front();
		XFreePacket(&pkt);
		packs.pop_front();
	}
	mux.unlock();
}
void XDecodeThread::Close()
{

}
//ȡ��һ֡���ݣ�����ջ�����û�з���NULL
AVPacket* XDecodeThread::Pop()
{

	mux.lock();
	if (packs.empty()) {
		mux.unlock();
		return NULL;
	}
	AVPacket* pkt = packs.front();
	packs.pop_front();
	mux.unlock();

	return pkt;
};
//����̣߳�������Ҫһ��list���н���push,Ȼ���߳�ȥ��AVPacket����
void XDecodeThread::Push(AVPacket* pkt)
{
	if (!pkt) return;
	//����
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
XDecodeThread::XDecodeThread() 
{
	//�򿪽�����
	if (!decode) decode = new XDecode();

}
 XDecodeThread::~XDecodeThread()
{
	 //�ȴ��߳��˳�
	 isExit = true;
	 wait();
}