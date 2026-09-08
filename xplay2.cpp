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
	startTimer(40);//40���룬һ���Ӱ�ʮ��֡
}

XPlay2::~XPlay2()
{
	//�ر�dt
	dt.Close();
}

void XPlay2::mouseDoubleClickEvent(QMouseEvent* e)
{
	if (isFullScreen()) {
		this->showNormal();
	}
	else {
		this->showFullScreen();
	}
}
//���ڳߴ�仯
void XPlay2::resizeEvent(QResizeEvent* e)
{
	ui.playPos->move(50, this->height() - 50);
	ui.playPos->resize(this->width() - 100, ui.playPos->height());
	ui.openFile->move(50, this->height() - 100);
	ui.isplay->move(ui.openFile->x() + ui.openFile->width() + 10, ui.openFile->y());
	ui.video->resize(this->size());

}

void XPlay2::PlayOrPause()
{
	bool isPause = !dt.isPause;
	SetPause(isPause);
	dt.SetPause(isPause);

}

void XPlay2::SetPause(bool isPause)
{
	if (isPause) {
		ui.isplay->setText(QStringLiteral("Play"));
	}
	else {
		ui.isplay->setText(QStringLiteral("Play"));
	}

}
//��ס������
void XPlay2::SliderPress()
{
	isSliderPress = true;

}
//�ɿ�������
void XPlay2::SliderRelease()
{
	isSliderPress = false;
	double pos = 0.0;
	pos = (double)ui.playPos->value() / (double)ui.playPos->maximum();
	dt.Seek(pos);
}

//һ����25֡�ǱȽ��ʺ����۵�
// ��ʱ����ʾ������
void XPlay2::timerEvent(QTimerEvent* e)
{
	if (isSliderPress) return;

	long long total = dt.totalMs;
	if (total > 0) {
		double pos = dt.pts / (double)total;
		int v = ui.playPos->maximum() * pos;
		ui.playPos->setValue(v);
	}

}

void XPlay2::OpenFile()
{
	//���ļ�
	QString name = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("ѡ����Ƶ�ļ�"));
	if (name.isEmpty()) return;
	this->setWindowTitle(name);
	if (!dt.Open(name.toLocal8Bit(), ui.video)) {
		QMessageBox::information(0, "error", "open file failed");
		return;
	}
	SetPause(dt.isPause);
	//qDebug() << name;
}

