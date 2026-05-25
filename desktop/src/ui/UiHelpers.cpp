#include "ui/UiHelpers.h"

#include <QVBoxLayout>

namespace
{
QPushButton *markRole(QPushButton *button, const char *role)
{
    button->setProperty("role", role);
    return button;
}
}

namespace UiHelpers
{

QFrame *createPageHeader(const QString &title, const QString &subtitle, QWidget *parent)
{
    auto *frame = new QFrame(parent);
    frame->setObjectName(QStringLiteral("pageHeader"));
    auto *layout = new QVBoxLayout(frame);
    layout->setContentsMargins(18, 14, 18, 14);
    layout->setSpacing(4);

    auto *titleLabel = new QLabel(title, frame);
    titleLabel->setObjectName(QStringLiteral("pageTitle"));
    layout->addWidget(titleLabel);

    if (!subtitle.isEmpty()) {
        auto *subtitleLabel = new QLabel(subtitle, frame);
        subtitleLabel->setObjectName(QStringLiteral("pageSubtitle"));
        subtitleLabel->setWordWrap(true);
        layout->addWidget(subtitleLabel);
    }
    return frame;
}

QFrame *createCard(QWidget *parent)
{
    auto *frame = new QFrame(parent);
    frame->setProperty("card", true);
    return frame;
}

QLabel *createSectionTitle(const QString &text, QWidget *parent)
{
    auto *label = new QLabel(text, parent);
    label->setProperty("sectionTitle", true);
    return label;
}

QLabel *createStatusLabel(QWidget *parent)
{
    auto *label = new QLabel(parent);
    label->setProperty("status", true);
    label->setWordWrap(true);
    return label;
}

QPushButton *markPrimary(QPushButton *button)
{
    return markRole(button, "primary");
}

QPushButton *markSecondary(QPushButton *button)
{
    return markRole(button, "secondary");
}

QPushButton *markDanger(QPushButton *button)
{
    return markRole(button, "danger");
}

QPushButton *markGhost(QPushButton *button)
{
    return markRole(button, "ghost");
}

void setButtonBusy(QPushButton *button, bool busy, const QString &busyText, const QString &idleText)
{
    if (!button) return;
    button->setEnabled(!busy);
    button->setText(busy ? busyText : idleText);
}

QString emptyStateText(const QString &context)
{
    if (context == QStringLiteral("myPosts")) {
        return QStringLiteral("还没有发布记录。可以先到“发布草稿”创建第一条需求。");
    }
    if (context == QStringLiteral("plaza")) {
        return QStringLiteral("暂时没有符合条件的需求。可以换个关键词或选择“全部”场景。");
    }
    if (context == QStringLiteral("conversations")) {
        return QStringLiteral("还没有会话。可以先在“广场”里找到合适的需求并发起联系。");
    }
    if (context == QStringLiteral("messages")) {
        return QStringLiteral("这段会话还没有消息，可以先发一句简短邀约。");
    }
    if (context == QStringLiteral("adminPosts")) {
        return QStringLiteral("当前没有待审核发布。");
    }
    if (context == QStringLiteral("adminIdentity")) {
        return QStringLiteral("当前没有待审核认证申请。");
    }
    if (context == QStringLiteral("givenReviews")) {
        return QStringLiteral("还没有发出评价。完成一次有效会话后再来评价对方。");
    }
    if (context == QStringLiteral("receivedReviews")) {
        return QStringLiteral("还没有收到评价。");
    }
    return QStringLiteral("暂无数据。");
}

QString sceneDisplayName(const QString &sceneType)
{
    if (sceneType == QStringLiteral("STUDY")) return QStringLiteral("学习自习");
    if (sceneType == QStringLiteral("MEAL")) return QStringLiteral("约饭");
    if (sceneType == QStringLiteral("SPORT")) return QStringLiteral("运动");
    if (sceneType == QStringLiteral("COURSE_TEAM")) return QStringLiteral("课程组队");
    if (sceneType == QStringLiteral("INNOVATION_PROJECT")) return QStringLiteral("创新项目");
    return sceneType;
}

QString statusDisplayName(const QString &status)
{
    if (status == QStringLiteral("DRAFT")) return QStringLiteral("草稿");
    if (status == QStringLiteral("PENDING_REVIEW")) return QStringLiteral("待审核");
    if (status == QStringLiteral("PUBLISHED")) return QStringLiteral("已发布");
    if (status == QStringLiteral("REJECTED")) return QStringLiteral("已驳回");
    if (status == QStringLiteral("ACTIVE")) return QStringLiteral("进行中");
    if (status == QStringLiteral("CLOSED")) return QStringLiteral("已关闭");
    if (status == QStringLiteral("VERIFIED")) return QStringLiteral("已认证");
    return status;
}

QString contactUnlockStatusText(const QString &status)
{
    if (status == QStringLiteral("UNLOCKED")) return QStringLiteral("已解锁，双方都可以查看联系方式");
    if (status == QStringLiteral("WAITING_FOR_PEER")) return QStringLiteral("已确认，等待对方确认");
    return QStringLiteral("未解锁，双方确认后才展示联系方式");
}

QString compactTags(const QStringList &tags)
{
    if (tags.isEmpty()) return QStringLiteral("暂无标签");
    return tags.join(QStringLiteral(" / "));
}

}
