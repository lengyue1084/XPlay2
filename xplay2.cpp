#include "xplay2.h"

#include "XAudioPlay.h"
#include "XDemuxThread.h"
#include "XSlider.h"
#include "XVideoWidget.h"

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFrame>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QSlider>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>
#include <QWindow>

namespace {
XDemuxThread demuxThread;
constexpr int kTimelineMaximum = 1000;

bool IsSupportedMedia(const QString& path)
{
    const QString suffix = QFileInfo(path).suffix().toLower();
    const QStringList supported = {QStringLiteral("mp4"), QStringLiteral("mkv"),
                                   QStringLiteral("avi"), QStringLiteral("mov"),
                                   QStringLiteral("flv"), QStringLiteral("wmv"),
                                   QStringLiteral("webm"), QStringLiteral("m4v")};
    return supported.contains(suffix);
}

QIcon TintedStandardIcon(QStyle* style, QStyle::StandardPixmap icon, const QColor& color)
{
    const QPixmap source = style->standardIcon(icon).pixmap(24, 24);
    QPixmap tinted(source.size());
    tinted.fill(Qt::transparent);
    QPainter painter(&tinted);
    painter.drawPixmap(0, 0, source);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(tinted.rect(), color);
    return QIcon(tinted);
}
}

XPlay2::XPlay2(QWidget* parent)
    : QWidget(parent)
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_StyledBackground, true);
    setAcceptDrops(true);
    setMinimumSize(1080, 680);
    resize(1400, 860);

    BuildUi();
    ApplyStyle();

    demuxThread.Start();
    XAudioPlay::Get()->SetVolume(0.68f);
    startTimer(40);
}

XPlay2::~XPlay2()
{
    demuxThread.Close();
}

QToolButton* XPlay2::CreateToolButton(QStyle::StandardPixmap icon, const QString& tooltip,
                                      const QString& objectName)
{
    auto* button = new QToolButton(this);
    button->setIcon(TintedStandardIcon(style(), icon, QColor(QStringLiteral("#d2d6db"))));
    button->setToolTip(tooltip);
    button->setCursor(Qt::PointingHandCursor);
    button->setAutoRaise(true);
    if (!objectName.isEmpty())
        button->setObjectName(objectName);
    return button;
}

void XPlay2::BuildUi()
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(1, 1, 1, 1);
    root->setSpacing(0);

    titleBar_ = new QFrame(this);
    titleBar_->setObjectName(QStringLiteral("titleBar"));
    titleBar_->setFixedHeight(58);
    titleBar_->installEventFilter(this);
    auto* titleLayout = new QHBoxLayout(titleBar_);
    titleLayout->setContentsMargins(18, 0, 10, 0);
    titleLayout->setSpacing(10);

    auto* brandButton = CreateToolButton(QStyle::SP_MediaPlay, tr("XPlay2"), QStringLiteral("brandButton"));
    brandButton->setFixedSize(29, 29);
    auto* brandLabel = new QLabel(QStringLiteral("XPlay2"), titleBar_);
    brandLabel->setObjectName(QStringLiteral("brandLabel"));
    titleLabel_ = new QLabel(tr("准备播放"), titleBar_);
    titleLabel_->setObjectName(QStringLiteral("titleLabel"));
    titleLabel_->setAlignment(Qt::AlignCenter);

    auto* minimizeButton = CreateToolButton(QStyle::SP_TitleBarMinButton, tr("最小化"));
    auto* maximizeButton = CreateToolButton(QStyle::SP_TitleBarMaxButton, tr("最大化"));
    auto* closeButton = CreateToolButton(QStyle::SP_TitleBarCloseButton, tr("关闭"), QStringLiteral("closeButton"));
    for (auto* button : {minimizeButton, maximizeButton, closeButton})
        button->setFixedSize(38, 34);

    titleLayout->addWidget(brandButton);
    titleLayout->addWidget(brandLabel);
    titleLayout->addStretch(1);
    titleLayout->addWidget(titleLabel_, 2);
    titleLayout->addStretch(1);
    titleLayout->addWidget(minimizeButton);
    titleLayout->addWidget(maximizeButton);
    titleLayout->addWidget(closeButton);
    root->addWidget(titleBar_);

    connect(minimizeButton, &QToolButton::clicked, this, &QWidget::showMinimized);
    connect(maximizeButton, &QToolButton::clicked, this, [this] {
        isMaximized() ? showNormal() : showMaximized();
    });
    connect(closeButton, &QToolButton::clicked, this, &QWidget::close);

    auto* body = new QHBoxLayout;
    body->setContentsMargins(0, 0, 0, 0);
    body->setSpacing(0);
    root->addLayout(body, 1);

    navigationRail_ = new QFrame(this);
    navigationRail_->setObjectName(QStringLiteral("navigationRail"));
    navigationRail_->setFixedWidth(68);
    auto* railLayout = new QVBoxLayout(navigationRail_);
    railLayout->setContentsMargins(12, 16, 12, 14);
    railLayout->setSpacing(9);

    auto* nowPlayingButton = CreateToolButton(QStyle::SP_MediaPlay, tr("正在播放"), QStringLiteral("railActive"));
    auto* openRailButton = CreateToolButton(QStyle::SP_DialogOpenButton, tr("打开媒体"));
    queueToggleButton_ = CreateToolButton(QStyle::SP_FileDialogDetailedView, tr("播放队列"));
    auto* infoButton = CreateToolButton(QStyle::SP_MessageBoxInformation, tr("播放器信息"));
    for (auto* button : {nowPlayingButton, openRailButton, queueToggleButton_, infoButton})
        button->setFixedSize(44, 44);

    railLayout->addWidget(nowPlayingButton);
    railLayout->addWidget(openRailButton);
    railLayout->addWidget(queueToggleButton_);
    railLayout->addStretch(1);
    railLayout->addWidget(infoButton);
    body->addWidget(navigationRail_);

    connect(openRailButton, &QToolButton::clicked, this, &XPlay2::OpenFile);
    connect(queueToggleButton_, &QToolButton::clicked, this, &XPlay2::ToggleQueue);
    connect(infoButton, &QToolButton::clicked, this, [this] {
        QMessageBox::information(this, tr("XPlay2"), tr("Qt 6 · FFmpeg · OpenGL\n现代桌面视频播放器"));
    });

    auto* center = new QFrame(this);
    center->setObjectName(QStringLiteral("centerPanel"));
    auto* centerLayout = new QVBoxLayout(center);
    centerLayout->setContentsMargins(16, 16, 16, 0);
    centerLayout->setSpacing(0);

    auto* videoFrame = new QFrame(center);
    videoFrame->setObjectName(QStringLiteral("videoFrame"));
    auto* videoLayout = new QVBoxLayout(videoFrame);
    videoLayout->setContentsMargins(0, 0, 0, 0);
    video_ = new XVideoWidget(videoFrame);
    video_->setObjectName(QStringLiteral("video"));
    video_->setMinimumSize(480, 270);
    video_->installEventFilter(this);
    videoLayout->addWidget(video_);

    emptyStateLabel_ = new QLabel(tr("将视频拖到这里\n或点击“添加文件”开始播放"), video_);
    emptyStateLabel_->setObjectName(QStringLiteral("emptyState"));
    emptyStateLabel_->setAlignment(Qt::AlignCenter);
    emptyStateLabel_->setAttribute(Qt::WA_TransparentForMouseEvents);
    emptyStateLabel_->adjustSize();
    emptyStateLabel_->raise();
    centerLayout->addWidget(videoFrame, 1);

    auto* controls = new QFrame(center);
    controls->setObjectName(QStringLiteral("controls"));
    controls->setFixedHeight(166);
    auto* controlsLayout = new QVBoxLayout(controls);
    controlsLayout->setContentsMargins(22, 17, 22, 17);
    controlsLayout->setSpacing(12);

    auto* timeline = new QHBoxLayout;
    timeline->setSpacing(12);
    currentTimeLabel_ = new QLabel(QStringLiteral("00:00"), controls);
    totalTimeLabel_ = new QLabel(QStringLiteral("00:00"), controls);
    currentTimeLabel_->setObjectName(QStringLiteral("timeLabel"));
    totalTimeLabel_->setObjectName(QStringLiteral("timeLabel"));
    totalTimeLabel_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    currentTimeLabel_->setFixedWidth(54);
    totalTimeLabel_->setFixedWidth(54);
    playPos_ = new XSlider(controls);
    playPos_->setObjectName(QStringLiteral("playPosition"));
    playPos_->setOrientation(Qt::Horizontal);
    playPos_->setRange(0, kTimelineMaximum);
    timeline->addWidget(currentTimeLabel_);
    timeline->addWidget(playPos_, 1);
    timeline->addWidget(totalTimeLabel_);
    controlsLayout->addLayout(timeline);

    auto* controlRow = new QHBoxLayout;
    controlRow->setSpacing(8);
    auto* volumeButton = CreateToolButton(QStyle::SP_MediaVolume, tr("静音"));
    volumeButton->setFixedSize(38, 38);
    volumeSlider_ = new QSlider(Qt::Horizontal, controls);
    volumeSlider_->setObjectName(QStringLiteral("volumeSlider"));
    volumeSlider_->setRange(0, 100);
    volumeSlider_->setValue(68);
    volumeSlider_->setFixedWidth(86);
    controlRow->addWidget(volumeButton);
    controlRow->addWidget(volumeSlider_);
    controlRow->addStretch(1);

    auto* backButton = CreateToolButton(QStyle::SP_MediaSkipBackward, tr("后退 10 秒"));
    playButton_ = CreateToolButton(QStyle::SP_MediaPlay, tr("播放/暂停"), QStringLiteral("playButton"));
    playButton_->setIcon(TintedStandardIcon(style(), QStyle::SP_MediaPlay,
                                            QColor(QStringLiteral("#111318"))));
    auto* forwardButton = CreateToolButton(QStyle::SP_MediaSkipForward, tr("前进 10 秒"));
    backButton->setFixedSize(42, 42);
    playButton_->setFixedSize(54, 54);
    forwardButton->setFixedSize(42, 42);
    controlRow->addWidget(backButton);
    controlRow->addWidget(playButton_);
    controlRow->addWidget(forwardButton);
    controlRow->addStretch(1);

    auto* openButton = new QPushButton(tr("添加文件"), controls);
    openButton->setObjectName(QStringLiteral("addButton"));
    openButton->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
    openButton->setCursor(Qt::PointingHandCursor);
    auto* fullScreenButton = CreateToolButton(QStyle::SP_TitleBarMaxButton, tr("全屏"));
    fullScreenButton->setFixedSize(40, 40);
    controlRow->addWidget(openButton);
    controlRow->addWidget(fullScreenButton);
    controlsLayout->addLayout(controlRow, 1);
    centerLayout->addWidget(controls);
    body->addWidget(center, 1);

    connect(playPos_, &QSlider::sliderPressed, this, &XPlay2::SliderPress);
    connect(playPos_, &QSlider::sliderReleased, this, &XPlay2::SliderRelease);
    connect(playButton_, &QToolButton::clicked, this, &XPlay2::PlayOrPause);
    connect(backButton, &QToolButton::clicked, this, [this] { SeekRelative(-10000); });
    connect(forwardButton, &QToolButton::clicked, this, [this] { SeekRelative(10000); });
    connect(openButton, &QPushButton::clicked, this, &XPlay2::OpenFile);
    connect(fullScreenButton, &QToolButton::clicked, this, &XPlay2::ToggleFullScreen);
    connect(volumeSlider_, &QSlider::valueChanged, this, [](int value) {
        XAudioPlay::Get()->SetVolume(value / 100.0f);
    });
    connect(volumeButton, &QToolButton::clicked, this, [this, volumeButton] {
        const bool mute = volumeSlider_->value() != 0;
        volumeSlider_->setValue(mute ? 0 : 68);
        volumeButton->setIcon(TintedStandardIcon(style(),
            mute ? QStyle::SP_MediaVolumeMuted : QStyle::SP_MediaVolume,
            QColor(QStringLiteral("#d2d6db"))));
    });

    queuePanel_ = new QFrame(this);
    queuePanel_->setObjectName(QStringLiteral("queuePanel"));
    queuePanel_->setFixedWidth(316);
    auto* queueLayout = new QVBoxLayout(queuePanel_);
    queueLayout->setContentsMargins(8, 0, 8, 14);
    queueLayout->setSpacing(0);

    auto* queueHeader = new QFrame(queuePanel_);
    queueHeader->setObjectName(QStringLiteral("queueHeader"));
    queueHeader->setFixedHeight(70);
    auto* queueHeaderLayout = new QHBoxLayout(queueHeader);
    queueHeaderLayout->setContentsMargins(10, 0, 6, 0);
    auto* queueTitle = new QLabel(tr("播放队列"), queueHeader);
    queueTitle->setObjectName(QStringLiteral("queueTitle"));
    queueCountLabel_ = new QLabel(tr("0 个视频"), queueHeader);
    queueCountLabel_->setObjectName(QStringLiteral("queueCount"));
    auto* closeQueueButton = CreateToolButton(QStyle::SP_TitleBarShadeButton, tr("收起队列"));
    closeQueueButton->setFixedSize(36, 36);
    queueHeaderLayout->addWidget(queueTitle);
    queueHeaderLayout->addWidget(queueCountLabel_);
    queueHeaderLayout->addStretch(1);
    queueHeaderLayout->addWidget(closeQueueButton);
    queueLayout->addWidget(queueHeader);

    queueList_ = new QListWidget(queuePanel_);
    queueList_->setObjectName(QStringLiteral("queueList"));
    queueList_->setIconSize(QSize(82, 48));
    queueList_->setSpacing(4);
    queueList_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    queueList_->setSelectionMode(QAbstractItemView::SingleSelection);
    queueLayout->addWidget(queueList_, 1);

    auto* queueAddButton = new QPushButton(tr("添加文件"), queuePanel_);
    queueAddButton->setObjectName(QStringLiteral("queueAddButton"));
    queueAddButton->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
    queueAddButton->setCursor(Qt::PointingHandCursor);
    queueLayout->addWidget(queueAddButton);
    body->addWidget(queuePanel_);

    connect(closeQueueButton, &QToolButton::clicked, this, &XPlay2::ToggleQueue);
    connect(queueAddButton, &QPushButton::clicked, this, &XPlay2::OpenFile);
    connect(queueList_, &QListWidget::itemDoubleClicked, this, &XPlay2::PlayQueueItem);
}

void XPlay2::ApplyStyle()
{
    setStyleSheet(QStringLiteral(R"(
        XPlay2 { background: #0c0e11; border: 1px solid #30343a; color: #f4f6f8; }
        QWidget { font-family: "Segoe UI", "Microsoft YaHei UI"; font-size: 13px; color: #f4f6f8; }
        QFrame#titleBar, QFrame#navigationRail { background: #111317; }
        QFrame#titleBar { border-bottom: 1px solid #2a2f36; }
        QFrame#navigationRail { border-right: 1px solid #2a2f36; }
        QLabel#brandLabel { font-size: 16px; font-weight: 700; }
        QLabel#titleLabel { color: #c6cbd1; }
        QToolButton { border: 0; border-radius: 5px; background: transparent; padding: 7px; }
        QToolButton:hover { background: #252a30; }
        QToolButton#brandButton { background: #ff5a4f; border-radius: 6px; padding: 5px; }
        QToolButton#closeButton:hover { background: #c42b1c; }
        QToolButton#railActive { background: #292e35; border-left: 3px solid #ff5a4f; }
        QFrame#centerPanel { background: #08090b; }
        QFrame#videoFrame, XVideoWidget#video { background: #020304; border-radius: 5px; }
        QLabel#emptyState { color: #9aa1aa; font-size: 15px; padding: 18px 26px; background: rgba(18, 21, 25, 190); border: 1px solid #343941; border-radius: 6px; }
        QFrame#controls { background: #14171b; border-top: 1px solid #252a30; }
        QLabel#timeLabel { color: #aab0b8; font-size: 12px; }
        QSlider::groove:horizontal { height: 4px; border-radius: 2px; background: #3a3f46; }
        QSlider#playPosition::sub-page:horizontal { background: #ff5a4f; border-radius: 2px; }
        QSlider#playPosition::handle:horizontal { width: 12px; margin: -5px 0; border-radius: 6px; background: white; }
        QSlider#volumeSlider::sub-page:horizontal { background: #42c6b7; border-radius: 2px; }
        QSlider#volumeSlider::handle:horizontal { width: 10px; margin: -4px 0; border-radius: 5px; background: #e8ecef; }
        QToolButton#playButton { background: white; border-radius: 27px; padding: 14px; }
        QToolButton#playButton:hover { background: #e8ebee; }
        QPushButton#addButton, QPushButton#queueAddButton { min-height: 36px; border: 1px solid #353a42; border-radius: 5px; background: #20242a; padding: 0 13px; }
        QPushButton#addButton:hover, QPushButton#queueAddButton:hover { background: #2a3037; border-color: #4a515b; }
        QFrame#queuePanel { background: #14171b; border-left: 1px solid #2a2f36; }
        QFrame#queueHeader { border-bottom: 1px solid #2a2f36; }
        QLabel#queueTitle { font-size: 15px; font-weight: 700; }
        QLabel#queueCount { color: #8e959f; font-size: 12px; }
        QListWidget#queueList { border: 0; outline: 0; background: transparent; padding-top: 8px; }
        QListWidget#queueList::item { min-height: 64px; border-radius: 6px; padding: 7px; color: #d7dbe0; }
        QListWidget#queueList::item:hover { background: #1e2228; }
        QListWidget#queueList::item:selected { background: #292e35; color: white; border-left: 3px solid #ff5a4f; }
        QScrollBar:vertical { width: 6px; background: transparent; }
        QScrollBar::handle:vertical { background: #3b4149; border-radius: 3px; min-height: 24px; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
        QToolTip { color: #f4f6f8; background: #252a30; border: 1px solid #3b424b; padding: 5px; }
    )"));
}

void XPlay2::OpenFile()
{
    const QString filePath = QFileDialog::getOpenFileName(
        this, tr("选择视频文件"), QString(),
        tr("视频文件 (*.mp4 *.mkv *.avi *.mov *.flv *.wmv *.webm *.m4v);;所有文件 (*.*)"));
    if (!filePath.isEmpty())
        OpenMedia(filePath);
}

bool XPlay2::OpenMedia(const QString& filePath)
{
    if (!QFileInfo::exists(filePath))
        return false;

    const QString absolutePath = QFileInfo(filePath).absoluteFilePath();
    const QByteArray nativePath = QFile::encodeName(absolutePath);
    if (!demuxThread.Open(nativePath.constData(), video_)) {
        QMessageBox::warning(this, tr("无法播放"), tr("无法打开该视频文件。"));
        return false;
    }

    currentFile_ = absolutePath;
    titleLabel_->setText(QFileInfo(filePath).completeBaseName());
    setWindowTitle(QStringLiteral("%1 - XPlay2").arg(QFileInfo(filePath).fileName()));
    emptyStateLabel_->hide();
    AddToQueue(currentFile_);
    SetPause(demuxThread.isPause);
    return true;
}

void XPlay2::AddToQueue(const QString& filePath)
{
    for (int i = 0; i < queueList_->count(); ++i) {
        if (queueList_->item(i)->data(Qt::UserRole).toString() == filePath) {
            queueList_->setCurrentRow(i);
            return;
        }
    }

    auto* item = new QListWidgetItem(style()->standardIcon(QStyle::SP_FileIcon),
                                     QFileInfo(filePath).completeBaseName(), queueList_);
    item->setData(Qt::UserRole, filePath);
    item->setToolTip(filePath);
    item->setSizeHint(QSize(0, 66));
    queueList_->setCurrentItem(item);
    queueCountLabel_->setText(tr("%1 个视频").arg(queueList_->count()));
}

void XPlay2::PlayQueueItem()
{
    if (auto* item = queueList_->currentItem())
        OpenMedia(item->data(Qt::UserRole).toString());
}

void XPlay2::PlayOrPause()
{
    if (currentFile_.isEmpty()) {
        OpenFile();
        return;
    }
    const bool pause = !demuxThread.isPause;
    demuxThread.SetPause(pause);
    SetPause(pause);
}

void XPlay2::SetPause(bool isPause)
{
    playButton_->setIcon(TintedStandardIcon(style(),
        isPause ? QStyle::SP_MediaPlay : QStyle::SP_MediaPause,
        QColor(QStringLiteral("#111318"))));
    playButton_->setToolTip(isPause ? tr("播放") : tr("暂停"));
}

void XPlay2::SliderPress()
{
    isSliderPress_ = true;
}

void XPlay2::SliderRelease()
{
    isSliderPress_ = false;
    if (demuxThread.totalMs > 0)
        demuxThread.Seek(playPos_->value() / static_cast<double>(playPos_->maximum()));
}

void XPlay2::SeekRelative(long long offsetMs)
{
    if (demuxThread.totalMs <= 0)
        return;
    const long long target = qBound(0LL, demuxThread.pts + offsetMs, demuxThread.totalMs);
    demuxThread.Seek(target / static_cast<double>(demuxThread.totalMs));
}

void XPlay2::timerEvent(QTimerEvent* event)
{
    QWidget::timerEvent(event);
    UpdateEmptyState();
    if (isSliderPress_ || demuxThread.totalMs <= 0)
        return;

    const long long current = qBound(0LL, demuxThread.pts, demuxThread.totalMs);
    playPos_->setValue(static_cast<int>(current * kTimelineMaximum / demuxThread.totalMs));
    currentTimeLabel_->setText(FormatTime(current));
    totalTimeLabel_->setText(FormatTime(demuxThread.totalMs));
}

QString XPlay2::FormatTime(long long milliseconds)
{
    const long long totalSeconds = qMax(0LL, milliseconds / 1000);
    const long long hours = totalSeconds / 3600;
    const long long minutes = (totalSeconds % 3600) / 60;
    const long long seconds = totalSeconds % 60;
    if (hours > 0)
        return QStringLiteral("%1:%2:%3").arg(hours, 2, 10, QLatin1Char('0'))
            .arg(minutes, 2, 10, QLatin1Char('0')).arg(seconds, 2, 10, QLatin1Char('0'));
    return QStringLiteral("%1:%2").arg(minutes, 2, 10, QLatin1Char('0'))
        .arg(seconds, 2, 10, QLatin1Char('0'));
}

void XPlay2::ToggleQueue()
{
    queuePanel_->setVisible(!queuePanel_->isVisible());
}

void XPlay2::ToggleFullScreen()
{
    const bool leaving = isFullScreen();
    leaving ? showNormal() : showFullScreen();
    titleBar_->setVisible(leaving);
    navigationRail_->setVisible(leaving);
    queuePanel_->setVisible(leaving);
}

void XPlay2::mouseDoubleClickEvent(QMouseEvent* event)
{
    ToggleFullScreen();
    event->accept();
}

bool XPlay2::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == titleBar_) {
        if (event->type() == QEvent::MouseButtonDblClick) {
            isMaximized() ? showNormal() : showMaximized();
            return true;
        }
        if (event->type() == QEvent::MouseButtonPress) {
            auto* mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton && windowHandle()) {
                windowHandle()->startSystemMove();
                return true;
            }
        }
    }

    if (watched == video_) {
        if (event->type() == QEvent::MouseButtonDblClick) {
            ToggleFullScreen();
            return true;
        }
        if (event->type() == QEvent::Resize)
            UpdateEmptyState();
    }
    return QWidget::eventFilter(watched, event);
}

void XPlay2::UpdateEmptyState()
{
    if (!emptyStateLabel_ || !video_ || emptyStateLabel_->isHidden())
        return;
    emptyStateLabel_->adjustSize();
    emptyStateLabel_->move((video_->size().width() - emptyStateLabel_->width()) / 2,
                           (video_->size().height() - emptyStateLabel_->height()) / 2);
    emptyStateLabel_->raise();
}

void XPlay2::dragEnterEvent(QDragEnterEvent* event)
{
    if (!event->mimeData()->hasUrls())
        return;
    const auto urls = event->mimeData()->urls();
    if (!urls.isEmpty() && urls.first().isLocalFile() && IsSupportedMedia(urls.first().toLocalFile()))
        event->acceptProposedAction();
}

void XPlay2::dropEvent(QDropEvent* event)
{
    const auto urls = event->mimeData()->urls();
    if (!urls.isEmpty() && OpenMedia(urls.first().toLocalFile()))
        event->acceptProposedAction();
}

void XPlay2::keyPressEvent(QKeyEvent* event)
{
    switch (event->key()) {
    case Qt::Key_Space:
        PlayOrPause();
        break;
    case Qt::Key_Left:
        SeekRelative(-10000);
        break;
    case Qt::Key_Right:
        SeekRelative(10000);
        break;
    case Qt::Key_F:
    case Qt::Key_F11:
        ToggleFullScreen();
        break;
    case Qt::Key_Escape:
        if (isFullScreen())
            ToggleFullScreen();
        else
            QWidget::keyPressEvent(event);
        break;
    case Qt::Key_O:
        if (event->modifiers() & Qt::ControlModifier)
            OpenFile();
        else
            QWidget::keyPressEvent(event);
        break;
    default:
        QWidget::keyPressEvent(event);
    }
}
