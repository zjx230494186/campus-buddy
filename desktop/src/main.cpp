#include <QApplication>
#include <QStackedWidget>
#include <QStringList>
#include <QTimer>

#include "api/CampusApiClient.h"
#include "auth/AuthApiService.h"
#include "auth/AuthTokenStore.h"
#include "domain/ApiClientConfig.h"
#include "ui/HomePageWidget.h"
#include "ui/LoginWidget.h"
#include "ui/RegisterWidget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    const ApiClientConfig config;
    CampusApiClient apiClient(config);
    AuthTokenStore tokenStore;
    AuthApiService authService(apiClient, tokenStore);

    QStackedWidget navigator;

    auto *loginPage = new LoginWidget(authService, &navigator);
    auto *registerPage = new RegisterWidget(authService, &navigator);
    auto *homePage = new HomePageWidget(authService, &navigator);

    navigator.addWidget(loginPage);
    navigator.addWidget(registerPage);
    navigator.addWidget(homePage);

    navigator.setCurrentIndex(0);

    QObject::connect(loginPage, &LoginWidget::loginSuccess, [&navigator]() {
        navigator.setCurrentIndex(2);
    });
    QObject::connect(loginPage, &LoginWidget::switchToRegister, [&navigator]() {
        navigator.setCurrentIndex(1);
    });
    QObject::connect(registerPage, &RegisterWidget::registerSuccess, [&navigator]() {
        navigator.setCurrentIndex(0);
    });
    QObject::connect(registerPage, &RegisterWidget::switchToLogin, [&navigator]() {
        navigator.setCurrentIndex(0);
    });
    QObject::connect(homePage, &HomePageWidget::logout, [&navigator, &tokenStore]() {
        tokenStore.clear();
        navigator.setCurrentIndex(0);
    });

    navigator.setWindowTitle(QStringLiteral("校园搭子平台"));
    navigator.resize(420, 480);
    navigator.show();

    if (QCoreApplication::arguments().contains("--smoke-test")) {
        QTimer::singleShot(0, &app, &QCoreApplication::quit);
    }

    return app.exec();
}
