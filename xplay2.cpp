#include "xplay2.h"
#include <QFileDialog>
#include <QDebug>
#include "XDemuxThread.h"
#include "QMessageBox"
static XDemuxThread dt;//ָ��ĺô������캯���ĵ��ò���Ӱ�쵽��ʼ��
XPlay2::XPlay2(QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	//dt = new XDemuxThread();
	dt.Start();
}

XPlay2::~XPlay2()
{}

void XPlay2::openFile()
{
	//���ļ�
	QString name = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("ѡ����Ƶ�ļ�"));
	if (name.isEmpty()) return;
	this->setWindowTitle(name);
	if (!dt.Open(name.toLocal8Bit(), ui.video)) {
		QMessageBox::information(0,"error","open file failed");
		return;
	}
	//qDebug() << name;
}

