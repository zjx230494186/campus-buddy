#include "ui/HomePageWidget.h"

#include <QVBoxLayout>

HomePageWidget::HomePageWidget(AuthApiService &authService, QWidget *parent)
    : QWidget(parent),
      authService_(authService)
{
    auto *layout = new QVBoxLayout(this);

    auto *title = new QLabel(QStringLiteral("校园搭子平台"), this);
    title->setAlignment(Qt::AlignCenter);
    QFont titleFont = title->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    title->setFont(titleFont);
    layout->addWidget(title);

    statusLabel_ = new QLabel(QStringLiteral("欢迎使用校园搭子平台"), this);
    statusLabel_->setAlignment(Qt::AlignCenter);
    layout->addWidget(statusLabel_);

    verificationStatusLabel_ = new QLabel(this);
    verificationStatusLabel_->setAlignment(Qt::AlignCenter);
    layout->addWidget(verificationStatusLabel_);

    checkStatusButton_ = new QPushButton(QStringLiteral("查询认证状态"), this);
    layout->addWidget(checkStatusButton_);

    logoutButton_ = new QPushButton(QStringLiteral("退出登录"), this);
    layout->addWidget(logoutButton_);

    layout->addStretch();

    connect(logoutButton_, &QPushButton::clicked, this, &HomePageWidget::onLogoutClicked);
    connect(checkStatusButton_, &QPushButton::clicked, this, &HomePageWidget::onCheckVerificationStatus);
}

void HomePageWidget::onLogoutClicked()
{
    emit logout();
}

void HomePageWidget::onCheckVerificationStatus()
{
    verificationStatusLabel_->setText(QStringLiteral("查询中..."));

    authService_.getIdentityVerificationStatus([this](const AuthResult &result) {
        if (result.success) {
            verificationStatusLabel_->setText(QStringLiteral("认证状态: %1").arg(result.authenticationStatus));
        } else {
            verificationStatusLabel_->setText(result.errorMessage.isEmpty() ? QStringLiteral("查询失败") : result.errorMessage);
        }
    });
}
