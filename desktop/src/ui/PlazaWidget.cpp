#include "ui/PlazaWidget.h"

#include <QVBoxLayout>

PlazaWidget::PlazaWidget(PartnerPostApiService &plazaService,
                         ContactConversationApiService &contactService,
                         QWidget *parent)
    : QWidget(parent),
      plazaService_(plazaService),
      contactService_(contactService)
{
    auto *layout = new QVBoxLayout(this);

    auto *header = new QLabel(QStringLiteral("广场"), this);
    header->setAlignment(Qt::AlignCenter);
    QFont f = header->font();
    f.setPointSize(14);
    f.setBold(true);
    header->setFont(f);
    layout->addWidget(header);

    auto *filterLayout = new QVBoxLayout();
    sceneTypeFilter_ = new QComboBox(this);
    sceneTypeFilter_->setObjectName(QStringLiteral("sceneTypeFilter"));
    sceneTypeFilter_->addItems({QStringLiteral("全部"), QStringLiteral("MEAL"), QStringLiteral("STUDY"), QStringLiteral("SPORT"),
                                QStringLiteral("COURSE_TEAM"), QStringLiteral("INNOVATION_PROJECT")});
    filterLayout->addWidget(sceneTypeFilter_);

    keywordFilter_ = new QLineEdit(this);
    keywordFilter_->setObjectName(QStringLiteral("keywordFilter"));
    keywordFilter_->setPlaceholderText(QStringLiteral("搜索关键词"));
    filterLayout->addWidget(keywordFilter_);
    layout->addLayout(filterLayout);

    refreshButton_ = new QPushButton(QStringLiteral("刷新广场"), this);
    refreshButton_->setObjectName(QStringLiteral("plazaRefreshButton"));
    layout->addWidget(refreshButton_);

    listWidget_ = new QListWidget(this);
    listWidget_->setObjectName(QStringLiteral("plazaListWidget"));
    layout->addWidget(listWidget_);

    detailLabel_ = new QLabel(this);
    detailLabel_->setObjectName(QStringLiteral("plazaDetailLabel"));
    detailLabel_->setWordWrap(true);
    layout->addWidget(detailLabel_);

    contactMessageEdit_ = new QLineEdit(this);
    contactMessageEdit_->setObjectName(QStringLiteral("contactMessageEdit"));
    contactMessageEdit_->setPlaceholderText(QStringLiteral("输入邀约消息后发起联系"));
    layout->addWidget(contactMessageEdit_);

    contactButton_ = new QPushButton(QStringLiteral("发起联系"), this);
    contactButton_->setObjectName(QStringLiteral("contactButton"));
    contactButton_->setEnabled(false);
    layout->addWidget(contactButton_);

    statusLabel_ = new QLabel(this);
    layout->addWidget(statusLabel_);

    connect(refreshButton_, &QPushButton::clicked, this, &PlazaWidget::onRefresh);
    connect(listWidget_, &QListWidget::currentRowChanged, this, &PlazaWidget::onItemSelected);
    connect(contactButton_, &QPushButton::clicked, this, &PlazaWidget::onContactRequest);
}

void PlazaWidget::onRefresh()
{
    statusLabel_->setText(QStringLiteral("加载中..."));
    QString sceneType = sceneTypeFilter_->currentText();
    if (sceneType == QStringLiteral("全部")) sceneType.clear();
    QString keyword = keywordFilter_->text();

    if (!sceneType.isEmpty() || !keyword.isEmpty()) {
        plazaService_.listPosts(sceneType, keyword, 0, 50, [this](const PlazaListResult &result) {
            if (result.success) {
                items_ = result.items;
                listWidget_->clear();
                for (const auto &item : items_) {
                    QString display = QStringLiteral("%1 [%2] by %3").arg(item.title, item.sceneType, item.publisherDisplayName);
                    listWidget_->addItem(display);
                }
                statusLabel_->setText(QStringLiteral("共 %1 条").arg(items_.size()));
            } else {
                statusLabel_->setText(QStringLiteral("加载失败: %1").arg(result.errorMessage));
            }
        });
    } else {
        plazaService_.listPosts(0, 50, [this](const PlazaListResult &result) {
            if (result.success) {
                items_ = result.items;
                listWidget_->clear();
                for (const auto &item : items_) {
                    QString display = QStringLiteral("%1 [%2] by %3").arg(item.title, item.sceneType, item.publisherDisplayName);
                    listWidget_->addItem(display);
                }
                statusLabel_->setText(QStringLiteral("共 %1 条").arg(items_.size()));
            } else {
                statusLabel_->setText(QStringLiteral("加载失败: %1").arg(result.errorMessage));
            }
        });
    }
}

void PlazaWidget::onItemSelected()
{
    selectedIndex_ = listWidget_->currentRow();
    if (selectedIndex_ < 0 || selectedIndex_ >= items_.size()) {
        detailLabel_->clear();
        contactButton_->setEnabled(false);
        return;
    }

    const auto &item = items_[selectedIndex_];
    detailLabel_->setText(QStringLiteral("加载详情..."));
    contactButton_->setEnabled(false);

    plazaService_.getPostDetail(item.postId, [this](const PlazaDetailResult &result) {
        if (result.success) {
            QString creditInfo;
            if (result.publisherCreditSummary.averageRating > 0) {
                creditInfo = QStringLiteral("  信用: %1星(%2评)").arg(result.publisherCreditSummary.averageRating).arg(result.publisherCreditSummary.ratingSampleCount);
            }
            detailLabel_->setText(
                QStringLiteral("标题: %1\n描述: %2\n发布者: %3 [%4]%5\n场景: %6\n时间: %7  地点: %8\n要求: %9\n标签: %10%11")
                    .arg(result.title)
                    .arg(result.description.left(200))
                    .arg(result.publisherDisplayName)
                    .arg(result.publisherAuthenticationStatus)
                    .arg(creditInfo)
                    .arg(result.sceneType)
                    .arg(result.timeText)
                    .arg(result.locationText)
                    .arg(result.targetRequirement)
                    .arg(result.tags.join(QStringLiteral(", ")))
                    .arg(result.ownPost ? QStringLiteral("\n(自己的发布)") : QString()));

            contactButton_->setEnabled(!result.ownPost);
            if (result.ownPost) {
                contactButton_->setText(QStringLiteral("不能联系自己的发布"));
            } else {
                contactButton_->setText(QStringLiteral("发起联系"));
            }
        } else {
            detailLabel_->setText(QStringLiteral("详情加载失败: %1 - %2").arg(result.errorCode).arg(result.errorMessage));
            contactButton_->setEnabled(false);
        }
    });
}

void PlazaWidget::onContactRequest()
{
    if (selectedIndex_ < 0 || selectedIndex_ >= items_.size()) return;
    const auto &item = items_[selectedIndex_];
    if (item.ownPost) return;

    QString message = contactMessageEdit_->text().trimmed();
    if (message.isEmpty()) {
        statusLabel_->setText(QStringLiteral("请输入邀约消息"));
        return;
    }

    statusLabel_->setText(QStringLiteral("联系中..."));
    contactService_.requestContact(item.postId, message, [this](const ContactRequestResult &result) {
        if (result.success) {
            statusLabel_->setText(QStringLiteral("联系成功! conversationId=%1").arg(result.conversationId));
            contactMessageEdit_->clear();
        } else {
            statusLabel_->setText(QStringLiteral("联系失败: %1 - %2").arg(result.errorCode).arg(result.errorMessage));
        }
    });
}
