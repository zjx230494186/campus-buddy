#include "ui/ReviewCreditWidget.h"

#include <QFormLayout>
#include <QVBoxLayout>

ReviewCreditWidget::ReviewCreditWidget(ReviewCreditApiService &reviewService, QWidget *parent)
    : QWidget(parent),
      reviewService_(reviewService)
{
    auto *layout = new QVBoxLayout(this);

    auto *header = new QLabel(QStringLiteral("评价与信用"), this);
    header->setAlignment(Qt::AlignCenter);
    QFont f = header->font(); f.setPointSize(14); f.setBold(true); header->setFont(f);
    layout->addWidget(header);

    creditSummaryLabel_ = new QLabel(this);
    creditSummaryLabel_->setObjectName(QStringLiteral("creditSummaryLabel"));
    creditSummaryLabel_->setWordWrap(true);
    layout->addWidget(creditSummaryLabel_);

    refreshCreditButton_ = new QPushButton(QStringLiteral("刷新信用摘要"), this);
    refreshCreditButton_->setObjectName(QStringLiteral("refreshCreditButton"));
    layout->addWidget(refreshCreditButton_);

    auto *submitForm = new QFormLayout();
    convIdEdit_ = new QLineEdit(this);
    convIdEdit_->setObjectName(QStringLiteral("convIdEdit"));
    convIdEdit_->setPlaceholderText(QStringLiteral("conversationId (long)"));
    submitForm->addRow(QStringLiteral("会话ID"), convIdEdit_);

    revieweeIdEdit_ = new QLineEdit(this);
    revieweeIdEdit_->setObjectName(QStringLiteral("revieweeIdEdit"));
    revieweeIdEdit_->setPlaceholderText(QStringLiteral("UUID"));
    submitForm->addRow(QStringLiteral("被评价者ID"), revieweeIdEdit_);

    ratingSpin_ = new QSpinBox(this);
    ratingSpin_->setObjectName(QStringLiteral("ratingSpin"));
    ratingSpin_->setRange(1, 6);
    ratingSpin_->setValue(5);
    submitForm->addRow(QStringLiteral("评分(1-6)"), ratingSpin_);

    reviewTagsEdit_ = new QLineEdit(this);
    reviewTagsEdit_->setObjectName(QStringLiteral("reviewTagsEdit"));
    reviewTagsEdit_->setPlaceholderText(QStringLiteral("逗号分隔, 如 friendly,punctual"));
    submitForm->addRow(QStringLiteral("评价标签"), reviewTagsEdit_);

    layout->addLayout(submitForm);

    submitReviewButton_ = new QPushButton(QStringLiteral("提交评价"), this);
    submitReviewButton_->setObjectName(QStringLiteral("submitReviewButton"));
    layout->addWidget(submitReviewButton_);

    auto *updateForm = new QFormLayout();
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
    layout->addLayout(updateForm);

    updateReviewButton_ = new QPushButton(QStringLiteral("修改评价"), this);
    updateReviewButton_->setObjectName(QStringLiteral("updateReviewButton"));
    layout->addWidget(updateReviewButton_);

    layout->addWidget(new QLabel(QStringLiteral("已发出评价:"), this));
    givenListWidget_ = new QListWidget(this);
    givenListWidget_->setObjectName(QStringLiteral("givenListWidget"));
    givenListWidget_->setMaximumHeight(120);
    layout->addWidget(givenListWidget_);
    refreshGivenButton_ = new QPushButton(QStringLiteral("刷新已发出"), this);
    layout->addWidget(refreshGivenButton_);

    layout->addWidget(new QLabel(QStringLiteral("已收到评价:"), this));
    receivedListWidget_ = new QListWidget(this);
    receivedListWidget_->setObjectName(QStringLiteral("receivedListWidget"));
    receivedListWidget_->setMaximumHeight(120);
    layout->addWidget(receivedListWidget_);
    refreshReceivedButton_ = new QPushButton(QStringLiteral("刷新已收到"), this);
    layout->addWidget(refreshReceivedButton_);

    statusLabel_ = new QLabel(this);
    layout->addWidget(statusLabel_);

    connect(refreshCreditButton_, &QPushButton::clicked, this, &ReviewCreditWidget::onRefreshCreditSummary);
    connect(submitReviewButton_, &QPushButton::clicked, this, &ReviewCreditWidget::onSubmitReview);
    connect(updateReviewButton_, &QPushButton::clicked, this, &ReviewCreditWidget::onUpdateReview);
    connect(refreshGivenButton_, &QPushButton::clicked, this, &ReviewCreditWidget::onRefreshGivenReviews);
    connect(refreshReceivedButton_, &QPushButton::clicked, this, &ReviewCreditWidget::onRefreshReceivedReviews);
}

void ReviewCreditWidget::onRefreshCreditSummary()
{
    statusLabel_->setText(QStringLiteral("加载信用摘要..."));
    reviewService_.getMyCreditSummary([this](const MyCreditSummaryResult &result) {
        if (result.success) {
            QString tagsStr;
            for (const auto &t : result.topTags) {
                if (!tagsStr.isEmpty()) tagsStr += QStringLiteral(", ");
                tagsStr += QStringLiteral("%1(%2)").arg(t.tag).arg(t.count);
            }
            creditSummaryLabel_->setText(
                QStringLiteral("评分: %1  真实会话: %2  评价样本: %3  争议: %4\nTopTags: %5")
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

    statusLabel_->setText(QStringLiteral("提交评价中..."));
    reviewService_.createReview(req, [this](const ReviewResult &result) {
        if (result.success) {
            statusLabel_->setText(QStringLiteral("评价已提交! reviewId=%1 status=%2 rating=%3")
                .arg(result.review.id).arg(result.review.status).arg(result.review.rating));
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

    statusLabel_->setText(QStringLiteral("修改评价中..."));
    reviewService_.updateReview(reviewId, req, [this](const ReviewResult &result) {
        if (result.success) {
            statusLabel_->setText(QStringLiteral("评价已修改! status=%1 rating=%2").arg(result.review.status).arg(result.review.rating));
        } else {
            statusLabel_->setText(QStringLiteral("修改失败: %1 - %2").arg(result.errorCode).arg(result.errorMessage));
        }
    });
}

void ReviewCreditWidget::onRefreshGivenReviews()
{
    statusLabel_->setText(QStringLiteral("加载已发出评价..."));
    reviewService_.listGivenReviews(0, 50, [this](const ReviewListResult &result) {
        if (result.success) {
            givenListWidget_->clear();
            for (const auto &item : result.items) {
                givenListWidget_->addItem(
                    QStringLiteral("id=%1 conv=%2 rating=%3 [%4] tags=%5")
                        .arg(item.id).arg(item.conversationId).arg(item.rating).arg(item.status)
                        .arg(item.reviewTags.join(QStringLiteral(","))));
            }
            statusLabel_->setText(QStringLiteral("已发出: %1 条").arg(result.items.size()));
        } else {
            statusLabel_->setText(QStringLiteral("加载失败: %1").arg(result.errorMessage));
        }
    });
}

void ReviewCreditWidget::onRefreshReceivedReviews()
{
    statusLabel_->setText(QStringLiteral("加载已收到评价..."));
    reviewService_.listReceivedReviews(0, 50, [this](const ReviewListResult &result) {
        if (result.success) {
            receivedListWidget_->clear();
            for (const auto &item : result.items) {
                receivedListWidget_->addItem(
                    QStringLiteral("id=%1 conv=%2 rating=%3 [%4] tags=%5")
                        .arg(item.id).arg(item.conversationId).arg(item.rating).arg(item.status)
                        .arg(item.reviewTags.join(QStringLiteral(","))));
            }
            statusLabel_->setText(QStringLiteral("已收到: %1 条").arg(result.items.size()));
        } else {
            statusLabel_->setText(QStringLiteral("加载失败: %1").arg(result.errorMessage));
        }
    });
}
