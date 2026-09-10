#pragma once

#include <QStyle>
#include <QWidget>

class QEvent;
class QFrame;
class QLabel;
class QListWidget;
class QSlider;
class QToolButton;
class XSlider;
class XVideoWidget;

class XPlay2 : public QWidget
{
    Q_OBJECT

public:
    explicit XPlay2(QWidget* parent = nullptr);
    ~XPlay2() override;
    bool OpenMedia(const QString& filePath);

protected:
    void timerEvent(QTimerEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private slots:
    void OpenFile();
    void PlayOrPause();
    void SliderPress();
    void SliderRelease();
    void ToggleFullScreen();
    void ToggleQueue();
    void PlayQueueItem();

private:
    void BuildUi();
    void ApplyStyle();
    void SetPause(bool isPause);
    void SeekRelative(long long offsetMs);
    void AddToQueue(const QString& filePath);
    void UpdateEmptyState();
    static QString FormatTime(long long milliseconds);
    QToolButton* CreateToolButton(QStyle::StandardPixmap icon, const QString& tooltip,
                                  const QString& objectName = QString());

    bool isSliderPress_ = false;
    QString currentFile_;

    QFrame* titleBar_ = nullptr;
    QFrame* navigationRail_ = nullptr;
    QFrame* queuePanel_ = nullptr;
    QLabel* titleLabel_ = nullptr;
    QLabel* currentTimeLabel_ = nullptr;
    QLabel* totalTimeLabel_ = nullptr;
    QLabel* emptyStateLabel_ = nullptr;
    QLabel* queueCountLabel_ = nullptr;
    XVideoWidget* video_ = nullptr;
    XSlider* playPos_ = nullptr;
    QSlider* volumeSlider_ = nullptr;
    QListWidget* queueList_ = nullptr;
    QToolButton* playButton_ = nullptr;
    QToolButton* queueToggleButton_ = nullptr;
};
