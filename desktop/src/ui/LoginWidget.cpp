#include "ui/LoginWidget.h"

#include <QFormLayout>
#include <QVBoxLayout>

LoginWidget::LoginWidget(AuthApiService &authService, QWidget *parent)
    : QWidget(parent),
      authService_(authService)
{
    auto *layout = new QVBoxLayout(this);

    auto *title = new QLabel(QStringLiteral("登录 - 校园搭子平台"), this);
    title->setAlignment(Qt::AlignCenter);
    QFont titleFont = title->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    title->setFont(titleFont);
    layout->addWidget(title);

    auto *formLayout = new QFormLayout();
    emailEdit_ = new QLineEdit(this);
    emailEdit_->setPlaceholderText(QStringLiteral("校园邮箱"));
    formLayout->addRow(QStringLiteral("邮箱:"), emailEdit_);

    passwordEdit_ = new QLineEdit(this);
    passwordEdit_->setEchoMode(QLineEdit::Password);
    passwordEdit_->setPlaceholderText(QStringLiteral("密码"));
    formLayout->addRow(QStringLiteral("密码:"), passwordEdit_);

    layout->addLayout(formLayout);

    loginButton_ = new QPushButton(QStringLiteral("登录"), this);
    layout->addWidget(loginButton_);

    registerButton_ = new QPushButton(QStringLiteral("没有账号？去注册"), this);
    registerButton_->setFlat(true);
    layout->addWidget(registerButton_);

    statusLabel_ = new QLabel(this);
    statusLabel_->setAlignment(Qt::AlignCenter);
    layout->addWidget(statusLabel_);

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
            emit loginSuccess();
        } else {
            statusLabel_->setText(result.errorMessage.isEmpty() ? QStringLiteral("登录失败") : result.errorMessage);
        }
    });
}
