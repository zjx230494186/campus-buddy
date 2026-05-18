#include "ui/RegisterWidget.h"

#include <QFormLayout>
#include <QVBoxLayout>

RegisterWidget::RegisterWidget(AuthApiService &authService, QWidget *parent)
    : QWidget(parent),
      authService_(authService)
{
    auto *layout = new QVBoxLayout(this);

    auto *title = new QLabel(QStringLiteral("注册 - 校园搭子平台"), this);
    title->setAlignment(Qt::AlignCenter);
    QFont titleFont = title->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    title->setFont(titleFont);
    layout->addWidget(title);

    auto *formLayout = new QFormLayout();
    realNameEdit_ = new QLineEdit(this);
    realNameEdit_->setPlaceholderText(QStringLiteral("真实姓名"));
    formLayout->addRow(QStringLiteral("姓名:"), realNameEdit_);

    studentNumberEdit_ = new QLineEdit(this);
    studentNumberEdit_->setPlaceholderText(QStringLiteral("学号"));
    formLayout->addRow(QStringLiteral("学号:"), studentNumberEdit_);

    emailEdit_ = new QLineEdit(this);
    emailEdit_->setPlaceholderText(QStringLiteral("校园邮箱"));
    formLayout->addRow(QStringLiteral("邮箱:"), emailEdit_);

    passwordEdit_ = new QLineEdit(this);
    passwordEdit_->setEchoMode(QLineEdit::Password);
    passwordEdit_->setPlaceholderText(QStringLiteral("密码"));
    formLayout->addRow(QStringLiteral("密码:"), passwordEdit_);

    codeEdit_ = new QLineEdit(this);
    codeEdit_->setPlaceholderText(QStringLiteral("验证码"));
    formLayout->addRow(QStringLiteral("验证码:"), codeEdit_);

    layout->addLayout(formLayout);

    sendCodeButton_ = new QPushButton(QStringLiteral("发送验证码"), this);
    layout->addWidget(sendCodeButton_);

    registerButton_ = new QPushButton(QStringLiteral("注册"), this);
    layout->addWidget(registerButton_);

    loginButton_ = new QPushButton(QStringLiteral("已有账号？去登录"), this);
    loginButton_->setFlat(true);
    layout->addWidget(loginButton_);

    statusLabel_ = new QLabel(this);
    statusLabel_->setAlignment(Qt::AlignCenter);
    layout->addWidget(statusLabel_);

    connect(sendCodeButton_, &QPushButton::clicked, this, &RegisterWidget::onSendCodeClicked);
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

void RegisterWidget::onRegisterClicked()
{
    const QString realName = realNameEdit_->text().trimmed();
    const QString studentNumber = studentNumberEdit_->text().trimmed();
    const QString email = emailEdit_->text().trimmed();
    const QString password = passwordEdit_->text();
    const QString code = codeEdit_->text().trimmed();

    if (realName.isEmpty() || studentNumber.isEmpty() || email.isEmpty() || password.isEmpty() || code.isEmpty()) {
        statusLabel_->setText(QStringLiteral("请填写所有字段"));
        return;
    }

    registerButton_->setEnabled(false);
    statusLabel_->setText(QStringLiteral("注册中..."));

    authService_.registerAccount(realName, studentNumber, email, password, code, [this](const AuthResult &result) {
        registerButton_->setEnabled(true);
        if (result.success) {
            statusLabel_->setText(QStringLiteral("注册成功，请登录"));
            emit registerSuccess();
        } else {
            statusLabel_->setText(result.errorMessage.isEmpty() ? QStringLiteral("注册失败") : result.errorMessage);
        }
    });
}
