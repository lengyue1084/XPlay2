#pragma once

struct AVPacket;
class XDecode;
#include <list>
#include <QThread>
#include "XDecode.h"
class XDecodeThread:public QThread
{
public:
	XDecodeThread();
	virtual ~XDecodeThread();
	//����̣߳�������Ҫһ��list���н���push,Ȼ���߳�ȥ��AVPacket����
	virtual void Push(AVPacket* pkt);

	//�������
	virtual void Clear();
	virtual void Close();

	//ȡ��һ֡���ݣ�����ջ�����û�з���NULL
	virtual AVPacket *Pop();

	//������
	//int maxList = 100 * 100;
	int maxList = 100;
	bool isExit = false;
protected:
	std::list<AVPacket *> packs;
	std::mutex mux;
	XDecode* decode = 0;
};

