#pragma once

#include <QtWidgets/QWidget>
#include "ui_xplay2.h"

class XPlay2 : public QWidget
{
    Q_OBJECT

public:
    //XPlay2(QWidget *parent = nullptr);
    XPlay2(QWidget* parent = Q_NULLPTR);
    ~XPlay2();

    //���ض�ʱ���ķ��� ��ʾ������
    void timerEvent(QTimerEvent * e);
    //���ڳߴ�仯
    void resizeEvent(QResizeEvent *e);
    //˫��ȫ��
    void mouseDoubleClickEvent(QMouseEvent *e);
    void SetPause(bool isPause);
public slots:
    void OpenFile();
    void PlayOrPause();
    void SliderPress();
    void SliderRelease();
   

private:
    //�������Ƿ�ס
    bool isSliderPress = false;
    Ui::XPlay2Class ui;
};
