#include "ui/MyPostsWidget.h"

#include <QVBoxLayout>

MyPostsWidget::MyPostsWidget(MyPartnerPostApiService &myPostService, QWidget *parent)
    : QWidget(parent),
      myPostService_(myPostService)
{
    auto *layout = new QVBoxLayout(this);

    auto *header = new QLabel(QStringLiteral("我的发布"), this);
    header->setAlignment(Qt::AlignCenter);
    QFont f = header->font();
    f.setPointSize(14);
    f.setBold(true);
    header->setFont(f);
    layout->addWidget(header);

    refreshButton_ = new QPushButton(QStringLiteral("刷新列表"), this);
    refreshButton_->setObjectName(QStringLiteral("myPostsRefreshButton"));
    layout->addWidget(refreshButton_);

    listWidget_ = new QListWidget(this);
    listWidget_->setObjectName(QStringLiteral("myPostsListWidget"));
    layout->addWidget(listWidget_);

    detailLabel_ = new QLabel(this);
    detailLabel_->setObjectName(QStringLiteral("myPostsDetailLabel"));
    detailLabel_->setWordWrap(true);
    layout->addWidget(detailLabel_);

    editButton_ = new QPushButton(QStringLiteral("编辑此发布"), this);
    editButton_->setObjectName(QStringLiteral("editPostButton"));
    editButton_->setEnabled(false);
    layout->addWidget(editButton_);

    withdrawButton_ = new QPushButton(QStringLiteral("撤回审核"), this);
    withdrawButton_->setObjectName(QStringLiteral("withdrawButton"));
    withdrawButton_->setEnabled(false);
    layout->addWidget(withdrawButton_);

    unpublishButton_ = new QPushButton(QStringLiteral("下架"), this);
    unpublishButton_->setObjectName(QStringLiteral("unpublishButton"));
    unpublishButton_->setEnabled(false);
    layout->addWidget(unpublishButton_);

    statusLabel_ = new QLabel(this);
    layout->addWidget(statusLabel_);

    connect(refreshButton_, &QPushButton::clicked, this, &MyPostsWidget::onRefresh);
    connect(listWidget_, &QListWidget::currentRowChanged, this, &MyPostsWidget::onItemSelected);
    connect(editButton_, &QPushButton::clicked, this, [this]() {
        if (selectedIndex_ >= 0 && selectedIndex_ < items_.size()) {
            const auto &item = items_[selectedIndex_];
            emit editPostRequested(item.postId, item);
        }
    });
    connect(withdrawButton_, &QPushButton::clicked, this, &MyPostsWidget::onWithdrawReview);
    connect(unpublishButton_, &QPushButton::clicked, this, &MyPostsWidget::onUnpublish);
}

void MyPostsWidget::onRefresh()
{
    statusLabel_->setText(QStringLiteral("加载中..."));
    myPostService_.listMyPosts(0, 50, [this](const MyPostListResult &result) {
        if (result.success) {
            items_ = result.items;
            listWidget_->clear();
            for (const auto &item : items_) {
                QString display = QStringLiteral("[%1] %2  %3").arg(item.status, item.title, item.sceneType);
                listWidget_->addItem(display);
            }
            statusLabel_->setText(QStringLiteral("共 %1 条").arg(items_.size()));
        } else {
            statusLabel_->setText(QStringLiteral("加载失败: %1").arg(result.errorMessage));
        }
    });
}

void MyPostsWidget::onItemSelected()
{
    selectedIndex_ = listWidget_->currentRow();
    if (selectedIndex_ < 0 || selectedIndex_ >= items_.size()) {
        detailLabel_->clear();
        editButton_->setEnabled(false);
        withdrawButton_->setEnabled(false);
        unpublishButton_->setEnabled(false);
        return;
    }

    const auto &item = items_[selectedIndex_];
    detailLabel_->setText(
        QStringLiteral("ID: %1\n标题: %2\n状态: %3\n场景: %4\n时间: %5\n地点: %6\n操作: %7")
            .arg(item.postId.left(8) + QStringLiteral("..."))
            .arg(item.title)
            .arg(item.status)
            .arg(item.sceneType)
            .arg(item.timeText)
            .arg(item.locationText)
            .arg(item.allowedActions.join(QStringLiteral(", "))));
    updateActionButtons();
}

void MyPostsWidget::updateActionButtons()
{
    if (selectedIndex_ < 0 || selectedIndex_ >= items_.size()) return;
    const auto &item = items_[selectedIndex_];
    editButton_->setEnabled(item.status == QStringLiteral("DRAFT") || item.allowedActions.contains(QStringLiteral("UPDATE_DRAFT")));
    withdrawButton_->setEnabled(item.allowedActions.contains(QStringLiteral("WITHDRAW_REVIEW")));
    unpublishButton_->setEnabled(item.allowedActions.contains(QStringLiteral("UNPUBLISH")));
}

void MyPostsWidget::onWithdrawReview()
{
    if (selectedIndex_ < 0 || selectedIndex_ >= items_.size()) return;
    const QString postId = items_[selectedIndex_].postId;
    statusLabel_->setText(QStringLiteral("撤回中..."));
    myPostService_.withdrawReview(postId, [this](const PostActionResult &result) {
        if (result.success) {
            statusLabel_->setText(QStringLiteral("已撤回审核"));
            onRefresh();
        } else {
            statusLabel_->setText(QStringLiteral("撤回失败: %1 - %2").arg(result.errorCode).arg(result.errorMessage));
        }
    });
}

void MyPostsWidget::onUnpublish()
{
    if (selectedIndex_ < 0 || selectedIndex_ >= items_.size()) return;
    const QString postId = items_[selectedIndex_].postId;
    statusLabel_->setText(QStringLiteral("下架中..."));
    myPostService_.unpublish(postId, [this](const PostActionResult &result) {
        if (result.success) {
            statusLabel_->setText(QStringLiteral("已下架"));
            onRefresh();
        } else {
            statusLabel_->setText(QStringLiteral("下架失败: %1 - %2").arg(result.errorCode).arg(result.errorMessage));
        }
    });
}
