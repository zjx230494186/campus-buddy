#include "ui/HomePageWidget.h"

#include <QVBoxLayout>

HomePageWidget::HomePageWidget(AuthApiService &authService,
                               MyPartnerPostApiService &myPostService,
                               PartnerPostApiService &plazaService,
                               ContactConversationApiService &contactService,
                               QWidget *parent)
    : QWidget(parent),
      authService_(authService)
{
    auto *layout = new QVBoxLayout(this);

    auto *title = new QLabel(QStringLiteral("校园搭子平台"), this);
    title->setAlignment(Qt::AlignCenter);
    QFont titleFont = title->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    title->setFont(titleFont);
    layout->addWidget(title);

    statusLabel_ = new QLabel(this);
    statusLabel_->setAlignment(Qt::AlignCenter);
    layout->addWidget(statusLabel_);

    verificationStatusLabel_ = new QLabel(this);
    verificationStatusLabel_->setAlignment(Qt::AlignCenter);
    layout->addWidget(verificationStatusLabel_);

    tabWidget_ = new QTabWidget(this);
    tabWidget_->setObjectName(QStringLiteral("mainTabWidget"));

    auto *authTab = new QWidget(tabWidget_);
    auto *authLayout = new QVBoxLayout(authTab);
    verificationWidget_ = new IdentityVerificationWidget(authService_, authTab);
    authLayout->addWidget(verificationWidget_);
    checkStatusButton_ = new QPushButton(QStringLiteral("查询认证状态"), authTab);
    authLayout->addWidget(checkStatusButton_);
    authLayout->addStretch();
    tabWidget_->addTab(authTab, QStringLiteral("认证"));

    postEditorWidget_ = new PostEditorWidget(myPostService, tabWidget_);
    tabWidget_->addTab(postEditorWidget_, QStringLiteral("发布草稿"));

    myPostsWidget_ = new MyPostsWidget(myPostService, tabWidget_);
    tabWidget_->addTab(myPostsWidget_, QStringLiteral("我的发布"));

    plazaWidget_ = new PlazaWidget(plazaService, contactService, tabWidget_);
    tabWidget_->addTab(plazaWidget_, QStringLiteral("广场"));

    conversationsWidget_ = new ConversationsWidget(contactService, tabWidget_);
    tabWidget_->addTab(conversationsWidget_, QStringLiteral("会话"));

    layout->addWidget(tabWidget_);

    logoutButton_ = new QPushButton(QStringLiteral("退出登录"), this);
    layout->addWidget(logoutButton_);

    connect(logoutButton_, &QPushButton::clicked, this, &HomePageWidget::onLogoutClicked);
    connect(checkStatusButton_, &QPushButton::clicked, this, &HomePageWidget::onCheckVerificationStatus);
    connect(myPostsWidget_, &MyPostsWidget::editPostRequested, this, &HomePageWidget::onEditPostRequested);
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
            verificationWidget_->setStatus(result.authenticationStatus, result.rejectReason);
        } else {
            verificationStatusLabel_->setText(result.errorMessage.isEmpty() ? QStringLiteral("查询失败") : result.errorMessage);
        }
    });
}

void HomePageWidget::onEditPostRequested(const QString &postId, const MyPostItem &item)
{
    postEditorWidget_->loadPost(postId, item);
    tabWidget_->setCurrentWidget(postEditorWidget_);
}
