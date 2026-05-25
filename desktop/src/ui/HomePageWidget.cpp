#include "ui/HomePageWidget.h"
#include "ui/UiHelpers.h"

#include <QHBoxLayout>
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
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(12);

    auto *header = UiHelpers::createPageHeader(
        QStringLiteral("校园搭子平台"),
        QStringLiteral("校园搭子、发布、联系、评价和审核的桌面演示端"),
        this);
    auto *headerRow = new QHBoxLayout();
    headerRow->setContentsMargins(0, 0, 0, 0);
    auto *headerText = new QWidget(header);
    auto *headerTextLayout = new QVBoxLayout(headerText);
    headerTextLayout->setContentsMargins(0, 0, 0, 0);
    welcomeLabel_ = new QLabel(this);
    welcomeLabel_->setProperty("muted", true);
    statusLabel_ = UiHelpers::createStatusLabel(this);
    verificationStatusLabel_ = UiHelpers::createStatusLabel(this);
    headerTextLayout->addWidget(welcomeLabel_);
    headerTextLayout->addWidget(statusLabel_);
    headerTextLayout->addWidget(verificationStatusLabel_);
    headerRow->addWidget(headerText, 1);
    headerRow->addStretch();
    logoutButton_ = UiHelpers::markDanger(new QPushButton(QStringLiteral("退出登录"), this));
    headerRow->addWidget(logoutButton_);
    qobject_cast<QVBoxLayout *>(header->layout())->addLayout(headerRow);
    layout->addWidget(header);

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

    checkStatusButton_ = UiHelpers::markSecondary(new QPushButton(QStringLiteral("查询认证状态"), this));

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
