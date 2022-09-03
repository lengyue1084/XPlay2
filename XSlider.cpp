#include "XSlider.h"
//从在鼠标时间，点击滑动条位置
void XSlider::mousePressEvent(QMouseEvent* e)
{
	double pos = (double)e->pos().x() / (double)width();//滑动条的百分比
	setValue(pos * this->maximum());
	//原有鼠标事件的处理,把事件继续向下传递
	//QSlider::mousePressEvent(e);
	QSlider::sliderReleased();//只调用这一个信号事件
}

XSlider::XSlider(QWidget *parent)
	: QSlider(parent)
{}

XSlider::~XSlider()
{}
