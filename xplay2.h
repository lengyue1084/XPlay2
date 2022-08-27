#pragma once

#include <QtWidgets/QWidget>
#include "ui_xplay2.h"

class XPlay2 : public QWidget
{
    Q_OBJECT

public:
    XPlay2(QWidget *parent = nullptr);
    ~XPlay2();
public slots:
    void openFile();
   

private:
    Ui::XPlay2Class ui;
};
