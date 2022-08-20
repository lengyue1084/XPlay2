#include "XAudioThread.h"
#include "XDecode.h"
#include "XAudioPlay.h"
#include "XResample.h"


bool XAudioThread::Open(AVCodecParameters* para)
{
	if (!para) return false;

	mux.lock();
	if (!decode) {
		decode = new XDecode();
	}
	if (!res) {
		res = new XResample();
	}
	if (!ap) {
		ap = XAudioPlay::Get();
	}


	mux.unlock();
	return true;

};
void XAudioThread::Run()
{

};
XAudioThread::XAudioThread()
{

};
XAudioThread::~XAudioThread()
{

};