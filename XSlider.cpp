#include "XSlider.h"
//�������ʱ�䣬���������λ��
void XSlider::mousePressEvent(QMouseEvent* e)
{
	double pos = (double)e->pos().x() / (double)width();//�������İٷֱ�
	setValue(pos * this->maximum());
	//ԭ������¼��Ĵ���,���¼��������´���
	//QSlider::mousePressEvent(e);
	QSlider::sliderReleased();//ֻ������һ���ź��¼�
}

XSlider::XSlider(QWidget *parent)
	: QSlider(parent)
{}

XSlider::~XSlider()
{}
