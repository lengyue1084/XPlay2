#include "xplay2.h"
#include <QtWidgets/QApplication>
#include <iostream>
using namespace std;
#include "XDemux.h"
#include "XDecode.h"
#include "XResample.h"
#include "XAudioPlay.h"
extern "C" {
#include "libavcodec/avcodec.h"
}
#include <QThread>
#include "XAudioThread.h"
#include "XVideoThread.h"
class TestThread : public QThread
{
public:
    XAudioThread at;
    XVideoThread vt;
    void Init()
    {
        const char* url = "rtmp://live.hkstv.hk.lxdns.com/hks";
        //const char* url = "rtmp://58.200.131.2:1935/livetv/gxtv";
        //cout << "demux.Open = " << demux.Open(url) << endl;
        demux.Read();
        demux.Clear();
        demux.Close();
        url = "v123.mp4";
        cout << "demux.Open = " << demux.Open(url) << endl;
        //cout << "demux.Seek = " << demux.Seek(0.9) << endl;
        //cout << "demux.CopyVPara = " << demux.CopyVPara() << endl;
        //cout << "demux.CopyAPara = " << demux.CopyAPara() << endl;
        //vdecode.Open(demux.CopyVPara());
        //adecode.Open(demux.CopyAPara());
        //�ز���
        //cout << "resample.Open = " << resample.Open(demux.CopyAPara()) << endl;;
       /* XAudioPlay::Get()->sampleRate = demux.sampleRate;
        XAudioPlay::Get()->channels = demux.channels;
        cout << "XAudioPlay::Get()->Open() = " << XAudioPlay::Get()->Open() << endl;;
        XAudioPlay::Get()->Open();*/

        cout << "at.Open = " << at.Open(demux.CopyAPara(),demux.sampleRate,demux.channels) << endl;
     ;
        vt.Open(demux.CopyVPara(), video, demux.width, demux.height);
        //��Ƶ���߳�����
        at.start();
        //������Ƶ�߳�
        vt.start();
    }
    unsigned char* pcm = new unsigned char[1024 * 1024];
    void run()
    {
        for (;;) {
            AVPacket* pkt = demux.Read();
            if (demux.IsAudio(pkt)) {
                at.Push(pkt);
               // //����
               // adecode.Send(pkt);
               // //���ս���
               // AVFrame* frame = adecode.Recv();
               // cout << "Audio = " << frame << endl;
               // //��Ƶ�ز���
               //int len = resample.Resample(frame,pcm);
               //cout << "�ز�����СResample��" << len << " ";
               //while (len > 0) {
               //    if (XAudioPlay::Get()->GetFree() >= len) {
               //        XAudioPlay::Get()->Write(pcm, len);
               //        break;
               //    }
               //    msleep(1);
               //}
                //av_frame_free(&frame);
            }
            else {
                vt.Push(pkt);
                /*vdecode.Send(pkt);
                AVFrame* frame = vdecode.Recv();
                video->Repaint(frame);
                cout << "Video = " << frame << endl;
                msleep(1);
                av_frame_free(&frame);*/
            }
            if (!pkt) break;
        }
    
    }
    // ����XDemux
    XDemux demux;
    /*XDecode vdecode;
    XDecode adecode;
    XResample resample*/
    XVideoWidget *video = 0;
};
#include "XDemuxThread.h"
//v123.mp4
int main(int argc, char *argv[])
{
 

  /*  vdecode.Clear();
    vdecode.Close();*/

  
    //TestThread tt;
    QApplication a(argc, argv);
    XPlay2 w;
    w.show();
    //��ʼ��gl����
    //w.ui.video->Init(tt.demux.width,tt.demux.height);
    //tt.video = w.ui.video;
    //tt.Init();
    //tt.start();


    XDemuxThread dt;
    dt.Open("v123.mp4",w.ui.video);
    dt.Start();
    return a.exec();
}
