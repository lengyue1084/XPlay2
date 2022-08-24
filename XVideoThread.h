#pragma once
#include <iostream>
#include <list>
#include <mutex>
#include <QThread>
#include "IVideoCall.h"
using namespace std;
struct AVPacket;
struct XDecode;
struct AVCodecParameters;
class XVideoThread : public QThread
{
public:
	//��,�ɹ����Ҫ������Դ
	virtual bool Open(AVCodecParameters* para, IVideoCall* call, int width, int herght);
	//����̣߳�������Ҫһ��list���н���push,Ȼ���߳�ȥ��AVPacket����
	virtual void Push(AVPacket* pkt);
	void run();


	XVideoThread();
	virtual ~XVideoThread();

	//������
	int maxList = 100 * 100;
	bool isExit = false;

protected:
	std::list<AVPacket*> packs;
	std::mutex mux;
	XDecode* decode = 0;
	IVideoCall* call = 0;
};

