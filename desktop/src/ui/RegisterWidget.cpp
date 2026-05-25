#include "ui/RegisterWidget.h"
#include "ui/UiHelpers.h"

#include <QFormLayout>
#include <QGroupBox>
#include <QVBoxLayout>

RegisterWidget::RegisterWidget(AuthApiService &authService, QWidget *parent)
    : QWidget(parent),
      authService_(authService)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(14);
    layout->addStretch();

    layout->addWidget(UiHelpers::createPageHeader(
        QStringLiteral("注册校园搭子平台"),
        QStringLiteral("使用校园邮箱完成验证码校验后创建账号。"),
        this));

    auto *formGroup = new QGroupBox(QStringLiteral("注册信息"), this);
    auto *formLayout = new QFormLayout(formGroup);
    emailEdit_ = new QLineEdit(this);
    emailEdit_->setPlaceholderText(QStringLiteral("校园邮箱"));
    formLayout->addRow(QStringLiteral("邮箱:"), emailEdit_);

    codeEdit_ = new QLineEdit(this);
    codeEdit_->setPlaceholderText(QStringLiteral("验证码"));
    formLayout->addRow(QStringLiteral("验证码:"), codeEdit_);

    displayNameEdit_ = new QLineEdit(this);
    displayNameEdit_->setPlaceholderText(QStringLiteral("显示名"));
    formLayout->addRow(QStringLiteral("显示名:"), displayNameEdit_);

    passwordEdit_ = new QLineEdit(this);
    passwordEdit_->setEchoMode(QLineEdit::Password);
    passwordEdit_->setPlaceholderText(QStringLiteral("密码"));
    formLayout->addRow(QStringLiteral("密码:"), passwordEdit_);

    layout->addWidget(formGroup);

    sendCodeButton_ = UiHelpers::markSecondary(new QPushButton(QStringLiteral("发送验证码"), this));
    layout->addWidget(sendCodeButton_);

    verifyCodeButton_ = UiHelpers::markSecondary(new QPushButton(QStringLiteral("校验验证码"), this));
    layout->addWidget(verifyCodeButton_);

    registerButton_ = UiHelpers::markPrimary(new QPushButton(QStringLiteral("注册"), this));
    layout->addWidget(registerButton_);

    loginButton_ = UiHelpers::markGhost(new QPushButton(QStringLiteral("已有账号？去登录"), this));
    loginButton_->setFlat(true);
    layout->addWidget(loginButton_);

    statusLabel_ = UiHelpers::createStatusLabel(this);
    statusLabel_->setAlignment(Qt::AlignCenter);
    layout->addWidget(statusLabel_);
    layout->addStretch();

    connect(sendCodeButton_, &QPushButton::clicked, this, &RegisterWidget::onSendCodeClicked);
    connect(verifyCodeButton_, &QPushButton::clicked, this, &RegisterWidget::onVerifyCodeClicked);
    connect(registerButton_, &QPushButton::clicked, this, &RegisterWidget::onRegisterClicked);
    connect(loginButton_, &QPushButton::clicked, this, &RegisterWidget::switchToLogin);
}

void RegisterWidget::onSendCodeClicked()
{
    const QString email = emailEdit_->text().trimmed();
    if (email.isEmpty()) {
        statusLabel_->setText(QStringLiteral("请先填写校园邮箱"));
        return;
    }

    sendCodeButton_->setEnabled(false);
    statusLabel_->setText(QStringLiteral("发送中..."));

    authService_.sendVerificationCode(email, [this](const AuthResult &result) {
        sendCodeButton_->setEnabled(true);
        if (result.success) {
            statusLabel_->setText(QStringLiteral("验证码已发送，请查收邮箱"));
        } else {
            statusLabel_->setText(result.errorMessage.isEmpty() ? QStringLiteral("发送失败") : result.errorMessage);
        }
    });
}

void RegisterWidget::onVerifyCodeClicked()
{
    const QString email = emailEdit_->text().trimmed();
    const QString code = codeEdit_->text().trimmed();
    if (email.isEmpty() || code.isEmpty()) {
        statusLabel_->setText(QStringLiteral("请填写邮箱和验证码"));
        return;
    }

    verifyCodeButton_->setEnabled(false);
    statusLabel_->setText(QStringLiteral("校验中..."));

    authService_.verifyCampusEmail(email, code, [this](const AuthResult &result) {
        verifyCodeButton_->setEnabled(true);
        if (result.success) {
            verificationTicket_ = result.verificationTicket;
            statusLabel_->setText(QStringLiteral("验证码校验成功"));
        } else {
            verificationTicket_.clear();
            statusLabel_->setText(result.errorMessage.isEmpty() ? QStringLiteral("校验失败") : result.errorMessage);
        }
    });
}

void RegisterWidget::onRegisterClicked()
{
    const QString email = emailEdit_->text().trimmed();
    const QString displayName = displayNameEdit_->text().trimmed();
    const QString password = passwordEdit_->text();

    if (email.isEmpty() || displayName.isEmpty() || password.isEmpty()) {
        statusLabel_->setText(QStringLiteral("请填写邮箱、显示名和密码"));
        return;
    }
    if (verificationTicket_.isEmpty()) {
        statusLabel_->setText(QStringLiteral("请先校验验证码"));
        return;
    }

    registerButton_->setEnabled(false);
    statusLabel_->setText(QStringLiteral("注册中..."));

    authService_.registerAccount(email, verificationTicket_, password, displayName, [this](const AuthResult &result) {
        registerButton_->setEnabled(true);
        if (result.success) {
            verificationTicket_.clear();
            statusLabel_->setText(QStringLiteral("注册成功，请登录"));
            emit registerSuccess();
        } else {
            statusLabel_->setText(result.errorMessage.isEmpty() ? QStringLiteral("注册失败") : result.errorMessage);
        }
    });
}
