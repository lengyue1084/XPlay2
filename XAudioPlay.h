#pragma once
class XAudioPlay
{

public:
	int sampleRate = 44100;//������
	int sampleSize = 16;//������С
	int channels = 2;
	//����Ƶ���ţ�����Ϊ���麯����ʵ�ַ��ڼ̳��൱��
	virtual bool Open() = 0;
	virtual void Close() = 0;
	//���ػ����л�û�в��ŵ�ʱ��ms
	virtual long long GetNoPlayMs() = 0;
	//������Ƶ
	virtual bool Write(const unsigned char* data, int datasize) = 0;
	//�ж��Ƿ����㹻�Ŀռ���д����Ƶ��Ϣ
	virtual int GetFree() = 0;

	//���ŵĻ�ֻ��һ������
	static XAudioPlay* Get();
	XAudioPlay();
	virtual ~XAudioPlay();
};

