#include <QApplication>
#include <QLabel>
#include <QStringList>
#include <QTimer>
#include <QWidget>

#include "domain/ApiClientConfig.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    const ApiClientConfig config;

    QWidget window;
    window.setWindowTitle("Campus Buddy Desktop");
    window.resize(420, 180);

    auto *label = new QLabel(QString("Campus Buddy Desktop\nAPI: %1").arg(config.apiBaseUrl()), &window);
    label->setAlignment(Qt::AlignCenter);
    label->setGeometry(window.rect());

    window.show();

    if (QCoreApplication::arguments().contains("--smoke-test")) {
        QTimer::singleShot(0, &app, &QCoreApplication::quit);
    }

    return app.exec();
}
