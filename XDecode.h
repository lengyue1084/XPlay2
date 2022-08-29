#pragma once
#include <mutex>
struct AVCodecParameters;
struct AVCodecContext;
struct AVFrame;
struct AVPacket;
extern void XFreePacket(AVPacket **pkt);
class XDecode
{
public:
	bool isAudio = false;
	//当前解码到的pts
	long long pts = 0;

	//打开解码器,不管是否成功并且释放空间，音视频通用
	virtual bool Open(AVCodecParameters *para);

	//发送到解码线程，不管陈宫与否清理pkt空间，对象和媒体内容空间
	virtual bool Send(AVPacket *pkt);
	//获取解码数据，一次send可能需要多次Recv,获取缓冲中的数据Send NULL再Recv中多次
	//每次赋值一份，由调用者释放av_frame_free
	virtual AVFrame* Recv();

	virtual void Close();
	virtual void Clear();

	XDecode();
	virtual ~XDecode();

	protected:
		AVCodecContext *codec = 0;
		//头文件中尽量不要使用命名空间，头文件不可控，不晓得是谁使用，可能造成冲突
		//命名空间防止同名函数冲突，cpp文件是自己可控的可以使用命名空间
		std::mutex mux;
	
};

