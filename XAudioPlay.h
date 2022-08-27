#pragma once
class XAudioPlay
{

public:
	int sampleRate = 44100;//样本率
	int sampleSize = 16;//样本大小
	int channels = 2;
	//打开音频播放，定义为纯虚函数，实现放在继承类当中
	virtual bool Open() = 0;
	virtual void Close() = 0;
	//返回缓存中还没有播放的时间ms
	virtual long long GetNoPlayMs() = 0;
	//播放音频
	virtual bool Write(const unsigned char* data, int datasize) = 0;
	//判断是否有足够的空间来写入音频信息
	virtual int GetFree() = 0;

	//播放的话只有一个对象
	static XAudioPlay* Get();
	XAudioPlay();
	virtual ~XAudioPlay();
};

