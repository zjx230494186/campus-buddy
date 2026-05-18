#pragma once

#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QWidget>

#include "auth/AuthApiService.h"

class RegisterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RegisterWidget(AuthApiService &authService, QWidget *parent = nullptr);

signals:
    void registerSuccess();
    void switchToLogin();

private slots:
    void onSendCodeClicked();
    void onRegisterClicked();

private:
    AuthApiService &authService_;

    QLineEdit *realNameEdit_;
    QLineEdit *studentNumberEdit_;
    QLineEdit *emailEdit_;
    QLineEdit *passwordEdit_;
    QLineEdit *codeEdit_;
    QPushButton *sendCodeButton_;
    QPushButton *registerButton_;
    QPushButton *loginButton_;
    QLabel *statusLabel_;
};
