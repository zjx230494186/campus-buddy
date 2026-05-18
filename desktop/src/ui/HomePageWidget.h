#pragma once

#include <QPushButton>
#include <QLabel>
#include <QWidget>

#include "auth/AuthApiService.h"

class HomePageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HomePageWidget(AuthApiService &authService, QWidget *parent = nullptr);

signals:
    void logout();

private slots:
    void onLogoutClicked();
    void onCheckVerificationStatus();

private:
    AuthApiService &authService_;
    QPushButton *logoutButton_;
    QPushButton *checkStatusButton_;
    QLabel *statusLabel_;
    QLabel *verificationStatusLabel_;
};
