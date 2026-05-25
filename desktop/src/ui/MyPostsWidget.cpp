#include "ui/MyPostsWidget.h"
#include "ui/UiHelpers.h"

#include <QGroupBox>
#include <QHBoxLayout>
#include <QSplitter>
#include <QVBoxLayout>

MyPostsWidget::MyPostsWidget(MyPartnerPostApiService &myPostService, QWidget *parent)
    : QWidget(parent),
      myPostService_(myPostService)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(12);

    layout->addWidget(UiHelpers::createPageHeader(
        QStringLiteral("我的发布"),
        QStringLiteral("管理草稿、审核中和已发布的搭子需求，必要时撤回或下架。"),
        this));

    refreshButton_ = UiHelpers::markSecondary(new QPushButton(QStringLiteral("刷新列表"), this));
    refreshButton_->setObjectName(QStringLiteral("myPostsRefreshButton"));
    layout->addWidget(refreshButton_);

    auto *splitter = new QSplitter(Qt::Horizontal, this);
    auto *listGroup = new QGroupBox(QStringLiteral("发布列表"), splitter);
    auto *listLayout = new QVBoxLayout(listGroup);

    listWidget_ = new QListWidget(this);
    listWidget_->setObjectName(QStringLiteral("myPostsListWidget"));
    listLayout->addWidget(listWidget_);

    auto *detailGroup = new QGroupBox(QStringLiteral("详情与操作"), splitter);
    auto *detailLayout = new QVBoxLayout(detailGroup);

    detailLabel_ = new QLabel(this);
    detailLabel_->setObjectName(QStringLiteral("myPostsDetailLabel"));
    detailLabel_->setWordWrap(true);
    detailLabel_->setText(QStringLiteral("请选择左侧发布查看详情。"));
    detailLayout->addWidget(detailLabel_, 1);

    auto *actionLayout = new QHBoxLayout();
    editButton_ = UiHelpers::markSecondary(new QPushButton(QStringLiteral("编辑"), this));
    editButton_->setObjectName(QStringLiteral("editPostButton"));
    editButton_->setEnabled(false);
    actionLayout->addWidget(editButton_);

    withdrawButton_ = UiHelpers::markSecondary(new QPushButton(QStringLiteral("撤回审核"), this));
    withdrawButton_->setObjectName(QStringLiteral("withdrawButton"));
    withdrawButton_->setEnabled(false);
    actionLayout->addWidget(withdrawButton_);

    unpublishButton_ = UiHelpers::markDanger(new QPushButton(QStringLiteral("下架"), this));
    unpublishButton_->setObjectName(QStringLiteral("unpublishButton"));
    unpublishButton_->setEnabled(false);
    actionLayout->addWidget(unpublishButton_);
    actionLayout->addStretch();
    detailLayout->addLayout(actionLayout);

    splitter->addWidget(listGroup);
    splitter->addWidget(detailGroup);
    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 3);
    layout->addWidget(splitter, 1);

    statusLabel_ = UiHelpers::createStatusLabel(this);
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
                QString display = QStringLiteral("%1\n%2 / %3")
                    .arg(item.title,
                         UiHelpers::statusDisplayName(item.status),
                         UiHelpers::sceneDisplayName(item.sceneType));
                listWidget_->addItem(display);
            }
            statusLabel_->setText(items_.isEmpty()
                ? QStringLiteral("还没有发布记录。可以先到“发布草稿”创建第一条需求。")
                : QStringLiteral("共 %1 条发布记录").arg(items_.size()));
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
        QStringLiteral("<b>%2</b><br><span style='color:#627d78'>%3 · %4</span><br><br>"
                       "<b>ID</b>：%1<br><b>时间</b>：%5<br><b>地点</b>：%6<br><b>可用操作</b>：%7")
            .arg(item.postId.left(8) + QStringLiteral("..."))
            .arg(item.title)
            .arg(UiHelpers::statusDisplayName(item.status))
            .arg(UiHelpers::sceneDisplayName(item.sceneType))
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
