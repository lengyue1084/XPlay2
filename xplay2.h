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

    //重载定时器的方法 显示滑动条
    void timerEvent(QTimerEvent * e);
    //窗口尺寸变化
    void resizeEvent(QResizeEvent *e);
    //双击全屏
    void mouseDoubleClickEvent(QMouseEvent *e);
    void SetPause(bool isPause);
public slots:
    void OpenFile();
    void PlayOrPause();
   

private:
    Ui::XPlay2Class ui;
};
