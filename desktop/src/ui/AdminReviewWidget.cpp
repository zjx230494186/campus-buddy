#include "ui/AdminReviewWidget.h"

#include <QFormLayout>
#include <QVBoxLayout>

AdminReviewWidget::AdminReviewWidget(AdminReviewApiService &adminService, QWidget *parent)
    : QWidget(parent),
      adminService_(adminService)
{
    auto *layout = new QVBoxLayout(this);

    auto *header = new QLabel(QStringLiteral("管理员审核"), this);
    header->setAlignment(Qt::AlignCenter);
    QFont f = header->font(); f.setPointSize(14); f.setBold(true); header->setFont(f);
    layout->addWidget(header);

    innerTab_ = new QTabWidget(this);

    auto *postTab = new QWidget(innerTab_);
    auto *postLayout = new QVBoxLayout(postTab);

    refreshPostQueueButton_ = new QPushButton(QStringLiteral("刷新发布审核队列"), postTab);
    postLayout->addWidget(refreshPostQueueButton_);

    postQueueList_ = new QListWidget(postTab);
    postQueueList_->setMaximumHeight(150);
    postLayout->addWidget(postQueueList_);

    postDetailLabel_ = new QLabel(postTab);
    postDetailLabel_->setWordWrap(true);
    postLayout->addWidget(postDetailLabel_);

    approvePostButton_ = new QPushButton(QStringLiteral("通过"), postTab);
    postLayout->addWidget(approvePostButton_);

    auto *rejectForm = new QFormLayout();
    rejectPostReasonEdit_ = new QLineEdit(postTab);
    rejectPostReasonEdit_->setPlaceholderText(QStringLiteral("驳回原因"));
    rejectForm->addRow(QStringLiteral("驳回原因"), rejectPostReasonEdit_);
    postLayout->addLayout(rejectForm);

    rejectPostButton_ = new QPushButton(QStringLiteral("驳回"), postTab);
    postLayout->addWidget(rejectPostButton_);

    innerTab_->addTab(postTab, QStringLiteral("发布审核"));

    auto *identityTab = new QWidget(innerTab_);
    auto *identityLayout = new QVBoxLayout(identityTab);

    refreshIdentityQueueButton_ = new QPushButton(QStringLiteral("刷新认证审核队列"), identityTab);
    identityLayout->addWidget(refreshIdentityQueueButton_);

    identityQueueList_ = new QListWidget(identityTab);
    identityQueueList_->setMaximumHeight(150);
    identityLayout->addWidget(identityQueueList_);

    identityDetailLabel_ = new QLabel(identityTab);
    identityDetailLabel_->setWordWrap(true);
    identityLayout->addWidget(identityDetailLabel_);

    approveIdentityButton_ = new QPushButton(QStringLiteral("通过"), identityTab);
    identityLayout->addWidget(approveIdentityButton_);

    auto *idRejectForm = new QFormLayout();
    rejectIdentityReasonEdit_ = new QLineEdit(identityTab);
    rejectIdentityReasonEdit_->setPlaceholderText(QStringLiteral("驳回原因"));
    idRejectForm->addRow(QStringLiteral("驳回原因"), rejectIdentityReasonEdit_);
    identityLayout->addLayout(idRejectForm);

    rejectIdentityButton_ = new QPushButton(QStringLiteral("驳回"), identityTab);
    identityLayout->addWidget(rejectIdentityButton_);

    innerTab_->addTab(identityTab, QStringLiteral("认证审核"));

    layout->addWidget(innerTab_);

    statusLabel_ = new QLabel(this);
    layout->addWidget(statusLabel_);

    connect(refreshPostQueueButton_, &QPushButton::clicked, this, &AdminReviewWidget::onRefreshPostQueue);
    connect(postQueueList_, &QListWidget::itemClicked, this, &AdminReviewWidget::onPostQueueItemClicked);
    connect(approvePostButton_, &QPushButton::clicked, this, &AdminReviewWidget::onApprovePost);
    connect(rejectPostButton_, &QPushButton::clicked, this, &AdminReviewWidget::onRejectPost);
    connect(refreshIdentityQueueButton_, &QPushButton::clicked, this, &AdminReviewWidget::onRefreshIdentityQueue);
    connect(approveIdentityButton_, &QPushButton::clicked, this, &AdminReviewWidget::onApproveIdentity);
    connect(rejectIdentityButton_, &QPushButton::clicked, this, &AdminReviewWidget::onRejectIdentity);
}

void AdminReviewWidget::onRefreshPostQueue()
{
    statusLabel_->setText(QStringLiteral("加载发布审核队列..."));
    adminService_.listPartnerPostReviewQueue(0, 50, [this](const PartnerPostReviewQueueResult &result) {
        if (result.success) {
            postQueueList_->clear();
            for (const auto &item : result.items) {
                postQueueList_->addItem(
                    QStringLiteral("[%1] %2 — %3")
                        .arg(item.status, item.publisherDisplayName, item.title));
            }
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

    adminService_.listPartnerPostReviewQueue(0, 50, [this, row](const PartnerPostReviewQueueResult &result) {
        if (result.success && row >= 0 && row < result.items.size()) {
            const auto &queueItem = result.items[row];
            selectedPostId_ = queueItem.postId;

            adminService_.getPartnerPostAdminDetail(selectedPostId_, [this](const AdminPostDetailResult &r) {
                if (r.success) {
                    postDetailLabel_->setText(
                        QStringLiteral("标题: %1\n场景: %2  状态: %3\n发布者: %4 (%5)\n描述: %6\n人数: %7  要求: %8\n标签: %9\n创建: %10")
                            .arg(r.detail.title, r.detail.sceneType, r.detail.status,
                                 r.detail.publisherDisplayName, r.detail.publisherAuthenticationStatus,
                                 r.detail.description,
                                 QString::number(r.detail.participantCount), r.detail.targetRequirement,
                                 r.detail.tags.join(QStringLiteral(",")),
                                 r.detail.createdAt));
                    statusLabel_->setText(QStringLiteral("已加载详情"));
                } else {
                    postDetailLabel_->setText(QString());
                    statusLabel_->setText(QStringLiteral("详情加载失败: %1").arg(r.errorMessage));
                }
            });
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
            identityQueueList_->clear();
            for (const auto &item : result.items) {
                identityQueueList_->addItem(
                    QStringLiteral("%1 %2 [%3] %4 %5 %6 — %7")
                        .arg(item.realName, item.studentNumber, item.reviewStatus,
                             item.college, item.major, item.grade,
                             item.materialContentType));
            }
            statusLabel_->setText(QStringLiteral("认证审核队列: %1 条").arg(result.items.size()));
        } else {
            statusLabel_->setText(QStringLiteral("加载失败: %1 - %2").arg(result.errorCode).arg(result.errorMessage));
        }
    });
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
        } else {
            statusLabel_->setText(QStringLiteral("驳回失败: %1 - %2").arg(r.errorCode).arg(r.errorMessage));
        }
    });
}
