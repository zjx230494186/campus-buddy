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
    void onVerifyCodeClicked();
    void onRegisterClicked();

private:
    AuthApiService &authService_;
    QString verificationTicket_;

    QLineEdit *emailEdit_;
    QLineEdit *codeEdit_;
    QLineEdit *displayNameEdit_;
    QLineEdit *passwordEdit_;
    QPushButton *sendCodeButton_;
    QPushButton *verifyCodeButton_;
    QPushButton *registerButton_;
    QPushButton *loginButton_;
    QLabel *statusLabel_;
};
