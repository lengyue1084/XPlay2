#pragma once

#include <QObject>
#include <QMouseEvent>
#include <QSlider>
class XSlider  : public QSlider
{
	Q_OBJECT

public:
	XSlider(QWidget *parent=NULL);
	~XSlider();
	//从在鼠标时间，点击滑动条位置
	void mousePressEvent(QMouseEvent * e);
};
