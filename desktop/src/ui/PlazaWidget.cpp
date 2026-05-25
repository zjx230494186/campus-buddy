#include "ui/PlazaWidget.h"
#include "ui/UiHelpers.h"

#include <QGroupBox>
#include <QHBoxLayout>
#include <QSplitter>
#include <QVBoxLayout>

PlazaWidget::PlazaWidget(PartnerPostApiService &plazaService,
                         ContactConversationApiService &contactService,
                         QWidget *parent)
    : QWidget(parent),
      plazaService_(plazaService),
      contactService_(contactService)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(12);

    layout->addWidget(UiHelpers::createPageHeader(
        QStringLiteral("搭子广场"),
        QStringLiteral("浏览已发布需求，查看发布者认证与信用摘要，再发起低压力联系。"),
        this));

    auto *filterCard = new QGroupBox(QStringLiteral("筛选"), this);
    auto *filterLayout = new QHBoxLayout(filterCard);
    sceneTypeFilter_ = new QComboBox(this);
    sceneTypeFilter_->setObjectName(QStringLiteral("sceneTypeFilter"));
    sceneTypeFilter_->addItems({QStringLiteral("全部"), QStringLiteral("MEAL"), QStringLiteral("STUDY"), QStringLiteral("SPORT"),
                                QStringLiteral("COURSE_TEAM"), QStringLiteral("INNOVATION_PROJECT")});
    filterLayout->addWidget(sceneTypeFilter_);

    keywordFilter_ = new QLineEdit(this);
    keywordFilter_->setObjectName(QStringLiteral("keywordFilter"));
    keywordFilter_->setPlaceholderText(QStringLiteral("搜索标题、描述或标签"));
    filterLayout->addWidget(keywordFilter_, 1);

    refreshButton_ = UiHelpers::markSecondary(new QPushButton(QStringLiteral("刷新广场"), this));
    refreshButton_->setObjectName(QStringLiteral("plazaRefreshButton"));
    filterLayout->addWidget(refreshButton_);
    layout->addWidget(filterCard);

    auto *splitter = new QSplitter(Qt::Horizontal, this);
    auto *listCard = new QGroupBox(QStringLiteral("需求列表"), splitter);
    auto *listLayout = new QVBoxLayout(listCard);

    listWidget_ = new QListWidget(this);
    listWidget_->setObjectName(QStringLiteral("plazaListWidget"));
    listLayout->addWidget(listWidget_);

    auto *detailCard = new QGroupBox(QStringLiteral("需求详情"), splitter);
    auto *detailLayout = new QVBoxLayout(detailCard);

    detailLabel_ = new QLabel(this);
    detailLabel_->setObjectName(QStringLiteral("plazaDetailLabel"));
    detailLabel_->setWordWrap(true);
    detailLabel_->setText(QStringLiteral("请选择左侧需求查看详情。"));
    detailLayout->addWidget(detailLabel_, 1);

    contactMessageEdit_ = new QLineEdit(this);
    contactMessageEdit_->setObjectName(QStringLiteral("contactMessageEdit"));
    contactMessageEdit_->setPlaceholderText(QStringLiteral("写一句自然的邀约消息"));
    detailLayout->addWidget(contactMessageEdit_);

    contactButton_ = UiHelpers::markPrimary(new QPushButton(QStringLiteral("发起联系"), this));
    contactButton_->setObjectName(QStringLiteral("contactButton"));
    contactButton_->setEnabled(false);
    detailLayout->addWidget(contactButton_);

    splitter->addWidget(listCard);
    splitter->addWidget(detailCard);
    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 3);
    layout->addWidget(splitter, 1);

    statusLabel_ = UiHelpers::createStatusLabel(this);
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
                    QString display = QStringLiteral("%1\n%2 / 发布者: %3")
                        .arg(item.title, UiHelpers::sceneDisplayName(item.sceneType), item.publisherDisplayName);
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
                    QString display = QStringLiteral("%1\n%2 / 发布者: %3")
                        .arg(item.title, UiHelpers::sceneDisplayName(item.sceneType), item.publisherDisplayName);
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
                QStringLiteral("<b>%1</b><br><span style='color:#627d78'>%2 · %3</span><br><br>"
                               "%4<br><br>"
                               "<b>发布者</b>：%5（%6）%7<br>"
                               "<b>要求</b>：%8<br>"
                               "<b>标签</b>：%9%10")
                    .arg(result.title.toHtmlEscaped())
                    .arg(UiHelpers::sceneDisplayName(result.sceneType).toHtmlEscaped())
                    .arg((result.timeText + QStringLiteral(" / ") + result.locationText).toHtmlEscaped())
                    .arg(result.description.left(260).toHtmlEscaped().replace(QStringLiteral("\n"), QStringLiteral("<br>")))
                    .arg(result.publisherDisplayName.toHtmlEscaped())
                    .arg(UiHelpers::statusDisplayName(result.publisherAuthenticationStatus).toHtmlEscaped())
                    .arg(creditInfo.toHtmlEscaped())
                    .arg(result.targetRequirement.toHtmlEscaped())
                    .arg(UiHelpers::compactTags(result.tags).toHtmlEscaped())
                    .arg(result.ownPost ? QStringLiteral("<br><b>这是你自己的发布，不能发起联系。</b>") : QString()));

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
            statusLabel_->setText(QStringLiteral("联系成功，已创建会话 #%1，可到“会话”页继续沟通。").arg(result.conversationId));
            contactMessageEdit_->clear();
        } else {
            statusLabel_->setText(QStringLiteral("联系失败: %1 - %2").arg(result.errorCode).arg(result.errorMessage));
        }
    });
}
