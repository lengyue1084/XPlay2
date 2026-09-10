#include "xplay2.h"

#include <QApplication>
#include <QTimer>

int main(int argc, char* argv[])
{
    QApplication application(argc, argv);
    application.setApplicationName(QStringLiteral("XPlay2"));
    application.setOrganizationName(QStringLiteral("XPlay2"));

    XPlay2 player;
    player.show();
    player.raise();
    player.activateWindow();

    const QStringList arguments = application.arguments();
    if (arguments.size() > 1) {
        const QString mediaPath = arguments.at(1);
        QTimer::singleShot(0, &player, [&player, mediaPath] {
            player.OpenMedia(mediaPath);
        });
    }
    return application.exec();
}
