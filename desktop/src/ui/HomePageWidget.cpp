#include "ui/HomePageWidget.h"

#include <QVBoxLayout>

HomePageWidget::HomePageWidget(AuthApiService &authService,
                               MyPartnerPostApiService &myPostService,
                               PartnerPostApiService &plazaService,
                               ContactConversationApiService &contactService,
                               ReviewCreditApiService &reviewService,
                               AdminReviewApiService &adminService,
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

    welcomeLabel_ = new QLabel(this);
    welcomeLabel_->setAlignment(Qt::AlignCenter);
    layout->addWidget(welcomeLabel_);

    statusLabel_ = new QLabel(this);
    statusLabel_->setAlignment(Qt::AlignCenter);
    layout->addWidget(statusLabel_);

    verificationStatusLabel_ = new QLabel(this);
    verificationStatusLabel_->setAlignment(Qt::AlignCenter);
    layout->addWidget(verificationStatusLabel_);

    tabWidget_ = new QTabWidget(this);
    tabWidget_->setObjectName(QStringLiteral("mainTabWidget"));

    verificationWidget_ = new IdentityVerificationWidget(authService_, this);
    postEditorWidget_ = new PostEditorWidget(myPostService, this);
    myPostsWidget_ = new MyPostsWidget(myPostService, this);
    plazaWidget_ = new PlazaWidget(plazaService, contactService, this);
    conversationsWidget_ = new ConversationsWidget(contactService, this);
    reviewCreditWidget_ = new ReviewCreditWidget(reviewService, this);
    adminReviewWidget_ = new AdminReviewWidget(adminService, this);

    layout->addWidget(tabWidget_);

    logoutButton_ = new QPushButton(QStringLiteral("退出登录"), this);
    layout->addWidget(logoutButton_);

    checkStatusButton_ = new QPushButton(QStringLiteral("查询认证状态"), this);

    connect(logoutButton_, &QPushButton::clicked, this, &HomePageWidget::onLogoutClicked);
    connect(checkStatusButton_, &QPushButton::clicked, this, &HomePageWidget::onCheckVerificationStatus);
    connect(myPostsWidget_, &MyPostsWidget::editPostRequested, this, &HomePageWidget::onEditPostRequested);
}

void HomePageWidget::setupTabsForRole(const QString &accountRole)
{
    currentRole_ = accountRole;
    while (tabWidget_->count() > 0) {
        tabWidget_->removeTab(0);
    }

    const bool isAdmin = (accountRole == QStringLiteral("ADMIN"));

    if (isAdmin) {
        welcomeLabel_->setText(QStringLiteral("管理员模式"));
        tabWidget_->addTab(adminReviewWidget_, QStringLiteral("发布审核"));
        tabWidget_->addTab(verificationWidget_, QStringLiteral("认证审核"));
    } else {
        welcomeLabel_->setText(QStringLiteral("学生模式"));

        auto *authTab = new QWidget(tabWidget_);
        auto *authLayout = new QVBoxLayout(authTab);
        authLayout->addWidget(verificationWidget_);
        authLayout->addWidget(checkStatusButton_);
        authLayout->addStretch();
        tabWidget_->addTab(authTab, QStringLiteral("认证"));

        tabWidget_->addTab(postEditorWidget_, QStringLiteral("发布草稿"));
        tabWidget_->addTab(myPostsWidget_, QStringLiteral("我的发布"));
        tabWidget_->addTab(plazaWidget_, QStringLiteral("广场"));
        tabWidget_->addTab(conversationsWidget_, QStringLiteral("会话"));
        tabWidget_->addTab(reviewCreditWidget_, QStringLiteral("评价信用"));
    }
}

void HomePageWidget::clearAllData()
{
    postEditorWidget_->clearForm();
    myPostsWidget_->onRefresh();
    plazaWidget_->onRefresh();
    conversationsWidget_->onRefreshConversations();
    adminReviewWidget_->onRefreshPostQueue();
    verificationStatusLabel_->clear();
    statusLabel_->clear();
    welcomeLabel_->clear();
}

void HomePageWidget::onLogoutClicked()
{
    clearAllData();
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
