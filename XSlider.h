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
	//�������ʱ�䣬���������λ��
	void mousePressEvent(QMouseEvent * e);
};
