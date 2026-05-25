#include "ui/ReviewCreditWidget.h"
#include "ui/UiHelpers.h"

#include <QFormLayout>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QVBoxLayout>

ReviewCreditWidget::ReviewCreditWidget(ReviewCreditApiService &reviewService, QWidget *parent)
    : QWidget(parent),
      reviewService_(reviewService)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(12);

    layout->addWidget(UiHelpers::createPageHeader(
        QStringLiteral("评价与信用"),
        QStringLiteral("查看自己的信用摘要，并在有效会话后提交或修改评价。"),
        this));

    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    auto *content = new QWidget(scrollArea);
    auto *contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(12);

    auto *summaryGroup = new QGroupBox(QStringLiteral("信用摘要"), content);
    auto *summaryLayout = new QVBoxLayout(summaryGroup);

    creditSummaryLabel_ = new QLabel(this);
    creditSummaryLabel_->setObjectName(QStringLiteral("creditSummaryLabel"));
    creditSummaryLabel_->setWordWrap(true);
    creditSummaryLabel_->setText(QStringLiteral("点击刷新后查看评分、真实会话数和高频标签。"));
    summaryLayout->addWidget(creditSummaryLabel_);

    refreshCreditButton_ = UiHelpers::markSecondary(new QPushButton(QStringLiteral("刷新信用摘要"), this));
    refreshCreditButton_->setObjectName(QStringLiteral("refreshCreditButton"));
    summaryLayout->addWidget(refreshCreditButton_);
    contentLayout->addWidget(summaryGroup);

    auto *submitGroup = new QGroupBox(QStringLiteral("提交评价"), content);
    auto *submitForm = new QFormLayout(submitGroup);
    convIdEdit_ = new QLineEdit(this);
    convIdEdit_->setObjectName(QStringLiteral("convIdEdit"));
    convIdEdit_->setPlaceholderText(QStringLiteral("从会话详情复制的会话 ID"));
    submitForm->addRow(QStringLiteral("会话ID"), convIdEdit_);

    revieweeIdEdit_ = new QLineEdit(this);
    revieweeIdEdit_->setObjectName(QStringLiteral("revieweeIdEdit"));
    revieweeIdEdit_->setPlaceholderText(QStringLiteral("UUID"));
    submitForm->addRow(QStringLiteral("被评价者ID"), revieweeIdEdit_);

    ratingSpin_ = new QSpinBox(this);
    ratingSpin_->setObjectName(QStringLiteral("ratingSpin"));
    ratingSpin_->setRange(1, 6);
    ratingSpin_->setValue(5);
    submitForm->addRow(QStringLiteral("评分"), ratingSpin_);

    reviewTagsEdit_ = new QLineEdit(this);
    reviewTagsEdit_->setObjectName(QStringLiteral("reviewTagsEdit"));
    reviewTagsEdit_->setPlaceholderText(QStringLiteral("逗号分隔，如 准时,友好,靠谱"));
    submitForm->addRow(QStringLiteral("评价标签"), reviewTagsEdit_);

    submitReviewButton_ = UiHelpers::markPrimary(new QPushButton(QStringLiteral("提交评价"), this));
    submitReviewButton_->setObjectName(QStringLiteral("submitReviewButton"));
    submitForm->addRow(QString(), submitReviewButton_);
    contentLayout->addWidget(submitGroup);

    auto *updateGroup = new QGroupBox(QStringLiteral("修改已提交评价"), content);
    auto *updateForm = new QFormLayout(updateGroup);
    reviewIdEdit_ = new QLineEdit(this);
    reviewIdEdit_->setObjectName(QStringLiteral("reviewIdEdit"));
    reviewIdEdit_->setPlaceholderText(QStringLiteral("reviewId (long)"));
    updateForm->addRow(QStringLiteral("评价ID"), reviewIdEdit_);

    updateRatingSpin_ = new QSpinBox(this);
    updateRatingSpin_->setObjectName(QStringLiteral("updateRatingSpin"));
    updateRatingSpin_->setRange(1, 6);
    updateRatingSpin_->setValue(5);
    updateForm->addRow(QStringLiteral("新评分"), updateRatingSpin_);

    updateTagsEdit_ = new QLineEdit(this);
    updateTagsEdit_->setObjectName(QStringLiteral("updateTagsEdit"));
    updateForm->addRow(QStringLiteral("新标签"), updateTagsEdit_);
    updateReviewButton_ = UiHelpers::markSecondary(new QPushButton(QStringLiteral("修改评价"), this));
    updateReviewButton_->setObjectName(QStringLiteral("updateReviewButton"));
    updateForm->addRow(QString(), updateReviewButton_);
    contentLayout->addWidget(updateGroup);

    auto *listLayout = new QHBoxLayout();
    auto *givenGroup = new QGroupBox(QStringLiteral("已发出评价"), content);
    auto *givenLayout = new QVBoxLayout(givenGroup);
    givenListWidget_ = new QListWidget(this);
    givenListWidget_->setObjectName(QStringLiteral("givenListWidget"));
    givenListWidget_->setMaximumHeight(120);
    givenLayout->addWidget(givenListWidget_);
    refreshGivenButton_ = UiHelpers::markSecondary(new QPushButton(QStringLiteral("刷新已发出"), this));
    givenLayout->addWidget(refreshGivenButton_);
    listLayout->addWidget(givenGroup);

    auto *receivedGroup = new QGroupBox(QStringLiteral("已收到评价"), content);
    auto *receivedLayout = new QVBoxLayout(receivedGroup);
    receivedListWidget_ = new QListWidget(this);
    receivedListWidget_->setObjectName(QStringLiteral("receivedListWidget"));
    receivedListWidget_->setMaximumHeight(120);
    receivedLayout->addWidget(receivedListWidget_);
    refreshReceivedButton_ = UiHelpers::markSecondary(new QPushButton(QStringLiteral("刷新已收到"), this));
    receivedLayout->addWidget(refreshReceivedButton_);
    listLayout->addWidget(receivedGroup);
    contentLayout->addLayout(listLayout, 1);
    contentLayout->addStretch();
    scrollArea->setWidget(content);
    layout->addWidget(scrollArea, 1);

    statusLabel_ = UiHelpers::createStatusLabel(this);
    layout->addWidget(statusLabel_);

    connect(refreshCreditButton_, &QPushButton::clicked, this, &ReviewCreditWidget::onRefreshCreditSummary);
    connect(submitReviewButton_, &QPushButton::clicked, this, &ReviewCreditWidget::onSubmitReview);
    connect(updateReviewButton_, &QPushButton::clicked, this, &ReviewCreditWidget::onUpdateReview);
    connect(refreshGivenButton_, &QPushButton::clicked, this, &ReviewCreditWidget::onRefreshGivenReviews);
    connect(refreshReceivedButton_, &QPushButton::clicked, this, &ReviewCreditWidget::onRefreshReceivedReviews);
}

void ReviewCreditWidget::onRefreshCreditSummary()
{
    UiHelpers::setButtonBusy(refreshCreditButton_, true, QStringLiteral("刷新中..."), QStringLiteral("刷新信用摘要"));
    statusLabel_->setText(QStringLiteral("加载信用摘要..."));
    reviewService_.getMyCreditSummary([this](const MyCreditSummaryResult &result) {
        UiHelpers::setButtonBusy(refreshCreditButton_, false, QStringLiteral("刷新中..."), QStringLiteral("刷新信用摘要"));
        if (result.success) {
            QString tagsStr;
            for (const auto &t : result.topTags) {
                if (!tagsStr.isEmpty()) tagsStr += QStringLiteral(", ");
                tagsStr += QStringLiteral("%1(%2)").arg(t.tag).arg(t.count);
            }
            creditSummaryLabel_->setText(
                QStringLiteral("<b>平均评分</b>：%1<br><b>真实会话</b>：%2<br><b>评价样本</b>：%3<br><b>争议评价</b>：%4<br><b>高频标签</b>：%5")
                    .arg(result.averageRating)
                    .arg(result.realConversationCount)
                    .arg(result.ratingSampleCount)
                    .arg(result.disputedReviewCount)
                    .arg(tagsStr));
            statusLabel_->setText(QStringLiteral("信用摘要已刷新"));
        } else {
            statusLabel_->setText(QStringLiteral("加载失败: %1 - %2").arg(result.errorCode).arg(result.errorMessage));
        }
    });
}

void ReviewCreditWidget::onSubmitReview()
{
    CreateReviewRequest req;
    req.conversationId = convIdEdit_->text().toLongLong();
    req.revieweeId = revieweeIdEdit_->text().trimmed();
    req.rating = ratingSpin_->value();
    req.reviewTags = reviewTagsEdit_->text().split(QStringLiteral(","), Qt::SkipEmptyParts);

    if (req.conversationId <= 0 || req.revieweeId.isEmpty()) {
        statusLabel_->setText(QStringLiteral("请输入 conversationId 和 revieweeId"));
        return;
    }

    UiHelpers::setButtonBusy(submitReviewButton_, true, QStringLiteral("提交中..."), QStringLiteral("提交评价"));
    statusLabel_->setText(QStringLiteral("提交评价中..."));
    reviewService_.createReview(req, [this](const ReviewResult &result) {
        UiHelpers::setButtonBusy(submitReviewButton_, false, QStringLiteral("提交中..."), QStringLiteral("提交评价"));
        if (result.success) {
            statusLabel_->setText(QStringLiteral("评价已提交，评分 %1，状态 %2。")
                .arg(result.review.rating).arg(result.review.status));
        } else {
            statusLabel_->setText(QStringLiteral("提交失败: %1 - %2").arg(result.errorCode).arg(result.errorMessage));
        }
    });
}

void ReviewCreditWidget::onUpdateReview()
{
    long long reviewId = reviewIdEdit_->text().toLongLong();
    if (reviewId <= 0) {
        statusLabel_->setText(QStringLiteral("请输入有效的 reviewId"));
        return;
    }

    UpdateReviewRequest req;
    req.rating = updateRatingSpin_->value();
    req.reviewTags = updateTagsEdit_->text().split(QStringLiteral(","), Qt::SkipEmptyParts);

    UiHelpers::setButtonBusy(updateReviewButton_, true, QStringLiteral("修改中..."), QStringLiteral("修改评价"));
    statusLabel_->setText(QStringLiteral("修改评价中..."));
    reviewService_.updateReview(reviewId, req, [this](const ReviewResult &result) {
        UiHelpers::setButtonBusy(updateReviewButton_, false, QStringLiteral("修改中..."), QStringLiteral("修改评价"));
        if (result.success) {
            statusLabel_->setText(QStringLiteral("评价已修改，评分 %1，状态 %2。").arg(result.review.rating).arg(result.review.status));
        } else {
            statusLabel_->setText(QStringLiteral("修改失败: %1 - %2").arg(result.errorCode).arg(result.errorMessage));
        }
    });
}

void ReviewCreditWidget::onRefreshGivenReviews()
{
    UiHelpers::setButtonBusy(refreshGivenButton_, true, QStringLiteral("加载中..."), QStringLiteral("刷新已发出"));
    statusLabel_->setText(QStringLiteral("加载已发出评价..."));
    reviewService_.listGivenReviews(0, 50, [this](const ReviewListResult &result) {
        UiHelpers::setButtonBusy(refreshGivenButton_, false, QStringLiteral("加载中..."), QStringLiteral("刷新已发出"));
        if (result.success) {
            givenListWidget_->clear();
            for (const auto &item : result.items) {
                givenListWidget_->addItem(
                    QStringLiteral("id=%1 conv=%2 rating=%3 [%4] tags=%5")
                        .arg(item.id).arg(item.conversationId).arg(item.rating).arg(item.status)
                        .arg(item.reviewTags.join(QStringLiteral(","))));
            }
            statusLabel_->setText(result.items.isEmpty()
                ? UiHelpers::emptyStateText(QStringLiteral("givenReviews"))
                : QStringLiteral("已发出: %1 条").arg(result.items.size()));
        } else {
            statusLabel_->setText(QStringLiteral("加载失败: %1").arg(result.errorMessage));
        }
    });
}

void ReviewCreditWidget::onRefreshReceivedReviews()
{
    UiHelpers::setButtonBusy(refreshReceivedButton_, true, QStringLiteral("加载中..."), QStringLiteral("刷新已收到"));
    statusLabel_->setText(QStringLiteral("加载已收到评价..."));
    reviewService_.listReceivedReviews(0, 50, [this](const ReviewListResult &result) {
        UiHelpers::setButtonBusy(refreshReceivedButton_, false, QStringLiteral("加载中..."), QStringLiteral("刷新已收到"));
        if (result.success) {
            receivedListWidget_->clear();
            for (const auto &item : result.items) {
                receivedListWidget_->addItem(
                    QStringLiteral("id=%1 conv=%2 rating=%3 [%4] tags=%5")
                        .arg(item.id).arg(item.conversationId).arg(item.rating).arg(item.status)
                        .arg(item.reviewTags.join(QStringLiteral(","))));
            }
            statusLabel_->setText(result.items.isEmpty()
                ? UiHelpers::emptyStateText(QStringLiteral("receivedReviews"))
                : QStringLiteral("已收到: %1 条").arg(result.items.size()));
        } else {
            statusLabel_->setText(QStringLiteral("加载失败: %1").arg(result.errorMessage));
        }
    });
}
