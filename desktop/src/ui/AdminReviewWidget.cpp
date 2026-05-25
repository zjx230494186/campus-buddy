#include "ui/AdminReviewWidget.h"
#include "ui/UiHelpers.h"

#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QSplitter>
#include <QVBoxLayout>

AdminReviewWidget::AdminReviewWidget(AdminReviewApiService &adminService, QWidget *parent)
    : QWidget(parent),
      adminService_(adminService)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(12);

    layout->addWidget(UiHelpers::createPageHeader(
        QStringLiteral("管理员审核"),
        QStringLiteral("集中处理发布审核和身份认证审核，先选择队列项，再查看详情并裁定。"),
        this));

    innerTab_ = new QTabWidget(this);

    auto *postTab = new QWidget(innerTab_);
    auto *postLayout = new QVBoxLayout(postTab);

    refreshPostQueueButton_ = UiHelpers::markSecondary(new QPushButton(QStringLiteral("刷新发布审核队列"), postTab));
    postLayout->addWidget(refreshPostQueueButton_);

    auto *postSplitter = new QSplitter(Qt::Horizontal, postTab);
    auto *postListGroup = new QGroupBox(QStringLiteral("待审核发布"), postSplitter);
    auto *postListLayout = new QVBoxLayout(postListGroup);

    postQueueList_ = new QListWidget(postTab);
    postListLayout->addWidget(postQueueList_);

    auto *postDetailGroup = new QGroupBox(QStringLiteral("发布详情"), postSplitter);
    auto *postDetailLayout = new QVBoxLayout(postDetailGroup);

    postDetailLabel_ = new QLabel(postTab);
    postDetailLabel_->setWordWrap(true);
    postDetailLabel_->setText(QStringLiteral("请选择左侧发布。"));
    postDetailLayout->addWidget(postDetailLabel_, 1);

    auto *postActionLayout = new QHBoxLayout();
    approvePostButton_ = UiHelpers::markPrimary(new QPushButton(QStringLiteral("通过"), postTab));
    approvePostButton_->setEnabled(false);
    postActionLayout->addWidget(approvePostButton_);

    auto *rejectForm = new QFormLayout();
    rejectPostReasonEdit_ = new QLineEdit(postTab);
    rejectPostReasonEdit_->setPlaceholderText(QStringLiteral("驳回原因"));
    rejectForm->addRow(QStringLiteral("驳回原因"), rejectPostReasonEdit_);
    postDetailLayout->addLayout(rejectForm);

    rejectPostButton_ = UiHelpers::markDanger(new QPushButton(QStringLiteral("驳回"), postTab));
    rejectPostButton_->setEnabled(false);
    postActionLayout->addWidget(rejectPostButton_);
    postActionLayout->addStretch();
    postDetailLayout->addLayout(postActionLayout);
    postSplitter->addWidget(postListGroup);
    postSplitter->addWidget(postDetailGroup);
    postSplitter->setStretchFactor(0, 2);
    postSplitter->setStretchFactor(1, 3);
    postLayout->addWidget(postSplitter, 1);

    innerTab_->addTab(postTab, QStringLiteral("发布审核"));

    auto *identityTab = new QWidget(innerTab_);
    auto *identityLayout = new QVBoxLayout(identityTab);

    refreshIdentityQueueButton_ = UiHelpers::markSecondary(new QPushButton(QStringLiteral("刷新认证审核队列"), identityTab));
    identityLayout->addWidget(refreshIdentityQueueButton_);

    auto *identitySplitter = new QSplitter(Qt::Horizontal, identityTab);
    auto *identityListGroup = new QGroupBox(QStringLiteral("待审核认证"), identitySplitter);
    auto *identityListLayout = new QVBoxLayout(identityListGroup);

    identityQueueList_ = new QListWidget(identityTab);
    identityListLayout->addWidget(identityQueueList_);

    auto *identityDetailGroup = new QGroupBox(QStringLiteral("认证详情"), identitySplitter);
    auto *identityDetailLayout = new QVBoxLayout(identityDetailGroup);

    identityDetailLabel_ = new QLabel(identityTab);
    identityDetailLabel_->setWordWrap(true);
    identityDetailLabel_->setText(QStringLiteral("请选择左侧认证申请。"));
    identityDetailLayout->addWidget(identityDetailLabel_, 1);

    auto *identityActionLayout = new QHBoxLayout();
    approveIdentityButton_ = UiHelpers::markPrimary(new QPushButton(QStringLiteral("通过"), identityTab));
    approveIdentityButton_->setEnabled(false);
    identityActionLayout->addWidget(approveIdentityButton_);

    auto *idRejectForm = new QFormLayout();
    rejectIdentityReasonEdit_ = new QLineEdit(identityTab);
    rejectIdentityReasonEdit_->setPlaceholderText(QStringLiteral("驳回原因"));
    idRejectForm->addRow(QStringLiteral("驳回原因"), rejectIdentityReasonEdit_);
    identityDetailLayout->addLayout(idRejectForm);

    rejectIdentityButton_ = UiHelpers::markDanger(new QPushButton(QStringLiteral("驳回"), identityTab));
    rejectIdentityButton_->setEnabled(false);
    identityActionLayout->addWidget(rejectIdentityButton_);
    identityActionLayout->addStretch();
    identityDetailLayout->addLayout(identityActionLayout);
    identitySplitter->addWidget(identityListGroup);
    identitySplitter->addWidget(identityDetailGroup);
    identitySplitter->setStretchFactor(0, 2);
    identitySplitter->setStretchFactor(1, 3);
    identityLayout->addWidget(identitySplitter, 1);

    innerTab_->addTab(identityTab, QStringLiteral("认证审核"));

    layout->addWidget(innerTab_);

    statusLabel_ = UiHelpers::createStatusLabel(this);
    layout->addWidget(statusLabel_);

    connect(refreshPostQueueButton_, &QPushButton::clicked, this, &AdminReviewWidget::onRefreshPostQueue);
    connect(postQueueList_, &QListWidget::itemClicked, this, &AdminReviewWidget::onPostQueueItemClicked);
    connect(approvePostButton_, &QPushButton::clicked, this, &AdminReviewWidget::onApprovePost);
    connect(rejectPostButton_, &QPushButton::clicked, this, &AdminReviewWidget::onRejectPost);
    connect(refreshIdentityQueueButton_, &QPushButton::clicked, this, &AdminReviewWidget::onRefreshIdentityQueue);
    connect(identityQueueList_, &QListWidget::itemClicked, this, &AdminReviewWidget::onIdentityQueueItemClicked);
    connect(approveIdentityButton_, &QPushButton::clicked, this, &AdminReviewWidget::onApproveIdentity);
    connect(rejectIdentityButton_, &QPushButton::clicked, this, &AdminReviewWidget::onRejectIdentity);
}

void AdminReviewWidget::onRefreshPostQueue()
{
    statusLabel_->setText(QStringLiteral("加载发布审核队列..."));
    adminService_.listPartnerPostReviewQueue(0, 50, [this](const PartnerPostReviewQueueResult &result) {
        if (result.success) {
            postQueueItems_ = result.items;
            selectedPostId_.clear();
            postQueueList_->clear();
            for (const auto &item : result.items) {
                postQueueList_->addItem(
                    QStringLiteral("%1\n%2 / %3")
                        .arg(item.title,
                             item.publisherDisplayName,
                             UiHelpers::statusDisplayName(item.status)));
            }
            approvePostButton_->setEnabled(false);
            rejectPostButton_->setEnabled(false);
            statusLabel_->setText(QStringLiteral("发布审核队列: %1 条").arg(result.items.size()));
        } else {
            statusLabel_->setText(QStringLiteral("加载失败: %1 - %2").arg(result.errorCode).arg(result.errorMessage));
        }
    });
}

void AdminReviewWidget::onPostQueueItemClicked(QListWidgetItem *item)
{
    if (!item) return;
    int row = postQueueList_->row(item);
    if (row < 0 || row >= postQueueItems_.size()) return;
    const auto &queueItem = postQueueItems_[row];
    selectedPostId_ = queueItem.postId;
    approvePostButton_->setEnabled(true);
    rejectPostButton_->setEnabled(true);

    adminService_.getPartnerPostAdminDetail(selectedPostId_, [this](const AdminPostDetailResult &r) {
        if (r.success) {
            postDetailLabel_->setText(
                QStringLiteral("<b>%1</b><br><span style='color:#627d78'>%2 · %3</span><br><br>"
                               "%4<br><br><b>发布者</b>：%5（%6）<br><b>人数</b>：%7<br><b>要求</b>：%8<br><b>标签</b>：%9<br><b>创建</b>：%10")
                    .arg(r.detail.title.toHtmlEscaped(),
                         UiHelpers::sceneDisplayName(r.detail.sceneType).toHtmlEscaped(),
                         UiHelpers::statusDisplayName(r.detail.status).toHtmlEscaped(),
                         r.detail.description.toHtmlEscaped().replace(QStringLiteral("\n"), QStringLiteral("<br>")),
                         r.detail.publisherDisplayName.toHtmlEscaped(),
                         UiHelpers::statusDisplayName(r.detail.publisherAuthenticationStatus).toHtmlEscaped(),
                         QString::number(r.detail.participantCount),
                         r.detail.targetRequirement.toHtmlEscaped(),
                         UiHelpers::compactTags(r.detail.tags).toHtmlEscaped(),
                         r.detail.createdAt.toHtmlEscaped()));
            statusLabel_->setText(QStringLiteral("已加载发布详情"));
        } else {
            postDetailLabel_->setText(QStringLiteral("详情加载失败"));
            statusLabel_->setText(QStringLiteral("详情加载失败: %1").arg(r.errorMessage));
        }
    });
}

void AdminReviewWidget::onApprovePost()
{
    if (selectedPostId_.isEmpty()) {
        statusLabel_->setText(QStringLiteral("请先选择一个帖子"));
        return;
    }
    PartnerPostReviewRequest req;
    req.decision = QStringLiteral("APPROVE");
    adminService_.reviewPartnerPost(selectedPostId_, req, [this](const PartnerPostReviewResult &r) {
        if (r.success) {
            statusLabel_->setText(QStringLiteral("审核通过! status=%1").arg(r.detail.status));
            selectedPostId_.clear();
            approvePostButton_->setEnabled(false);
            rejectPostButton_->setEnabled(false);
            onRefreshPostQueue();
        } else {
            statusLabel_->setText(QStringLiteral("审核失败: %1 - %2").arg(r.errorCode).arg(r.errorMessage));
        }
    });
}

void AdminReviewWidget::onRejectPost()
{
    if (selectedPostId_.isEmpty()) {
        statusLabel_->setText(QStringLiteral("请先选择一个帖子"));
        return;
    }
    PartnerPostReviewRequest req;
    req.decision = QStringLiteral("REJECT");
    req.reason = rejectPostReasonEdit_->text().trimmed();
    if (req.reason.isEmpty()) {
        statusLabel_->setText(QStringLiteral("驳回必须填写原因"));
        return;
    }
    adminService_.reviewPartnerPost(selectedPostId_, req, [this](const PartnerPostReviewResult &r) {
        if (r.success) {
            statusLabel_->setText(QStringLiteral("已驳回! status=%1 reason=%2").arg(r.detail.status).arg(r.detail.rejectReason));
            selectedPostId_.clear();
            approvePostButton_->setEnabled(false);
            rejectPostButton_->setEnabled(false);
            onRefreshPostQueue();
        } else {
            statusLabel_->setText(QStringLiteral("驳回失败: %1 - %2").arg(r.errorCode).arg(r.errorMessage));
        }
    });
}

void AdminReviewWidget::onRefreshIdentityQueue()
{
    statusLabel_->setText(QStringLiteral("加载认证审核队列..."));
    adminService_.listPendingIdentityVerifications(0, 50, [this](const PendingIdentityVerificationListResult &result) {
        if (result.success) {
            identityQueueItems_ = result.items;
            selectedSubmissionId_.clear();
            identityQueueList_->clear();
            for (const auto &item : result.items) {
                identityQueueList_->addItem(
                    QStringLiteral("%1 · %2\n%3 / %4 %5 %6")
                        .arg(item.realName,
                             item.studentNumber,
                             UiHelpers::statusDisplayName(item.reviewStatus),
                             item.college,
                             item.major,
                             item.grade));
            }
            approveIdentityButton_->setEnabled(false);
            rejectIdentityButton_->setEnabled(false);
            statusLabel_->setText(QStringLiteral("认证审核队列: %1 条").arg(result.items.size()));
        } else {
            statusLabel_->setText(QStringLiteral("加载失败: %1 - %2").arg(result.errorCode).arg(result.errorMessage));
        }
    });
}

void AdminReviewWidget::onIdentityQueueItemClicked(QListWidgetItem *item)
{
    if (!item) return;
    const int row = identityQueueList_->row(item);
    if (row < 0 || row >= identityQueueItems_.size()) return;

    const auto &selected = identityQueueItems_[row];
    selectedSubmissionId_ = selected.submissionId;
    approveIdentityButton_->setEnabled(true);
    rejectIdentityButton_->setEnabled(true);
    identityDetailLabel_->setText(
        QStringLiteral("<b>%1</b><br><span style='color:#627d78'>%2 · %3</span><br><br>"
                       "<b>学院专业</b>：%4 / %5 / %6<br><b>材料</b>：%7，%8 KB<br><b>提交时间</b>：%9")
            .arg(selected.realName.toHtmlEscaped(),
                 selected.studentNumber.toHtmlEscaped(),
                 UiHelpers::statusDisplayName(selected.reviewStatus).toHtmlEscaped(),
                 selected.college.toHtmlEscaped(),
                 selected.major.toHtmlEscaped(),
                 selected.grade.toHtmlEscaped(),
                 selected.materialContentType.toHtmlEscaped(),
                 QString::number(selected.materialSizeBytes / 1024),
                 selected.submittedAt.toHtmlEscaped()));
    statusLabel_->setText(QStringLiteral("已选择认证申请"));
}

void AdminReviewWidget::onApproveIdentity()
{
    if (selectedSubmissionId_.isEmpty()) {
        statusLabel_->setText(QStringLiteral("请先选择一条认证申请"));
        return;
    }
    IdentityVerificationReviewRequest req;
    req.decision = QStringLiteral("APPROVED");
    adminService_.reviewIdentityVerification(selectedSubmissionId_, req, [this](const IdentityVerificationReviewResult &r) {
        if (r.success) {
            statusLabel_->setText(QStringLiteral("认证通过! reviewStatus=%1 authStatus=%2").arg(r.reviewStatus).arg(r.authenticationStatus));
            selectedSubmissionId_.clear();
            approveIdentityButton_->setEnabled(false);
            rejectIdentityButton_->setEnabled(false);
            onRefreshIdentityQueue();
        } else {
            statusLabel_->setText(QStringLiteral("认证审核失败: %1 - %2").arg(r.errorCode).arg(r.errorMessage));
        }
    });
}

void AdminReviewWidget::onRejectIdentity()
{
    if (selectedSubmissionId_.isEmpty()) {
        statusLabel_->setText(QStringLiteral("请先选择一条认证申请"));
        return;
    }
    IdentityVerificationReviewRequest req;
    req.decision = QStringLiteral("REJECTED");
    req.rejectReason = rejectIdentityReasonEdit_->text().trimmed();
    if (req.rejectReason.isEmpty()) {
        statusLabel_->setText(QStringLiteral("驳回必须填写原因"));
        return;
    }
    adminService_.reviewIdentityVerification(selectedSubmissionId_, req, [this](const IdentityVerificationReviewResult &r) {
        if (r.success) {
            statusLabel_->setText(QStringLiteral("已驳回! reviewStatus=%1").arg(r.reviewStatus));
            selectedSubmissionId_.clear();
            approveIdentityButton_->setEnabled(false);
            rejectIdentityButton_->setEnabled(false);
            onRefreshIdentityQueue();
        } else {
            statusLabel_->setText(QStringLiteral("驳回失败: %1 - %2").arg(r.errorCode).arg(r.errorMessage));
        }
    });
}
