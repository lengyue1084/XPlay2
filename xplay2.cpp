#include "xplay2.h"
#include <QFileDialog>
#include <QDebug>
#include "XDemuxThread.h"
#include "QMessageBox"
static XDemuxThread dt;//指针的好处，构造函数的调用不会影响到初始化
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
	//打开文件
	QString name = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("选择视频文件"));
	if (name.isEmpty()) return;
	this->setWindowTitle(name);
	if (!dt.Open(name.toLocal8Bit(), ui.video)) {
		QMessageBox::information(0,"error","open file failed");
		return;
	}
	//qDebug() << name;
}

