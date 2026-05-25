#include "ui/LoginWidget.h"
#include "ui/UiHelpers.h"

#include <QFormLayout>
#include <QGroupBox>
#include <QVBoxLayout>

LoginWidget::LoginWidget(AuthApiService &authService, QWidget *parent)
    : QWidget(parent),
      authService_(authService)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(14);
    layout->addStretch();

    layout->addWidget(UiHelpers::createPageHeader(
        QStringLiteral("登录校园搭子平台"),
        QStringLiteral("进入发布、广场、会话和评价演示链路。"),
        this));

    auto *formGroup = new QGroupBox(QStringLiteral("账号信息"), this);
    auto *formLayout = new QFormLayout(formGroup);
    emailEdit_ = new QLineEdit(this);
    emailEdit_->setPlaceholderText(QStringLiteral("校园邮箱"));
    formLayout->addRow(QStringLiteral("邮箱:"), emailEdit_);

    passwordEdit_ = new QLineEdit(this);
    passwordEdit_->setEchoMode(QLineEdit::Password);
    passwordEdit_->setPlaceholderText(QStringLiteral("密码"));
    formLayout->addRow(QStringLiteral("密码:"), passwordEdit_);

    layout->addWidget(formGroup);

    loginButton_ = UiHelpers::markPrimary(new QPushButton(QStringLiteral("登录"), this));
    layout->addWidget(loginButton_);

    registerButton_ = UiHelpers::markGhost(new QPushButton(QStringLiteral("没有账号？去注册"), this));
    registerButton_->setFlat(true);
    layout->addWidget(registerButton_);

    statusLabel_ = UiHelpers::createStatusLabel(this);
    statusLabel_->setAlignment(Qt::AlignCenter);
    layout->addWidget(statusLabel_);
    layout->addStretch();

    connect(loginButton_, &QPushButton::clicked, this, &LoginWidget::onLoginClicked);
    connect(registerButton_, &QPushButton::clicked, this, &LoginWidget::switchToRegister);
}

void LoginWidget::onLoginClicked()
{
    const QString email = emailEdit_->text().trimmed();
    const QString password = passwordEdit_->text();

    if (email.isEmpty() || password.isEmpty()) {
        statusLabel_->setText(QStringLiteral("请填写邮箱和密码"));
        return;
    }

    loginButton_->setEnabled(false);
    statusLabel_->setText(QStringLiteral("登录中..."));

    authService_.login(email, password, [this](const AuthResult &result) {
        loginButton_->setEnabled(true);
        if (result.success) {
            statusLabel_->setText(QStringLiteral("登录成功"));
            emit loginSuccess(result.accountRole);
        } else {
            statusLabel_->setText(result.errorMessage.isEmpty() ? QStringLiteral("登录失败") : result.errorMessage);
        }
    });
}
