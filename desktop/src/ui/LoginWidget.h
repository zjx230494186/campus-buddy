#pragma once

#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QWidget>

#include "auth/AuthApiService.h"

class LoginWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWidget(AuthApiService &authService, QWidget *parent = nullptr);

signals:
    void loginSuccess(const QString &accountRole);
    void switchToRegister();

private slots:
    void onLoginClicked();

private:
    AuthApiService &authService_;

    QLineEdit *emailEdit_;
    QLineEdit *passwordEdit_;
    QPushButton *loginButton_;
    QPushButton *registerButton_;
    QLabel *statusLabel_;
};
