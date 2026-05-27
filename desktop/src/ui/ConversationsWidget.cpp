#include "ui/ConversationsWidget.h"
#include "ui/UiHelpers.h"

#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QSplitter>
#include <QVBoxLayout>

ConversationsWidget::ConversationsWidget(ContactConversationApiService &contactService, QWidget *parent)
    : QWidget(parent),
      contactService_(contactService)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(12);

    layout->addWidget(UiHelpers::createPageHeader(
        QStringLiteral("会话与联系方式"),
        QStringLiteral("左侧选择会话，右侧完成消息沟通、已读、关闭和双方联系方式交换。"),
        this));

    auto *splitter = new QSplitter(Qt::Horizontal, this);
    auto *listCard = new QGroupBox(QStringLiteral("会话列表"), splitter);
    auto *listLayout = new QVBoxLayout(listCard);

    refreshButton_ = UiHelpers::markSecondary(new QPushButton(QStringLiteral("刷新会话列表"), this));
    refreshButton_->setObjectName(QStringLiteral("refreshConversationsButton"));
    listLayout->addWidget(refreshButton_);

    conversationListWidget_ = new QListWidget(this);
    conversationListWidget_->setObjectName(QStringLiteral("conversationListWidget"));
    listLayout->addWidget(conversationListWidget_, 1);

    conversationStatusLabel_ = UiHelpers::createStatusLabel(this);
    conversationStatusLabel_->setObjectName(QStringLiteral("conversationStatusLabel"));
    listLayout->addWidget(conversationStatusLabel_);

    auto *rightPanel = new QWidget(splitter);
    auto *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(12);

    auto *messageCard = new QGroupBox(QStringLiteral("消息区"), rightPanel);
    auto *messageLayout = new QVBoxLayout(messageCard);

    messageListWidget_ = new QListWidget(this);
    messageListWidget_->setObjectName(QStringLiteral("messageListWidget"));
    messageLayout->addWidget(messageListWidget_, 1);

    messageEdit_ = new QLineEdit(this);
    messageEdit_->setObjectName(QStringLiteral("messageEdit"));
    messageEdit_->setPlaceholderText(QStringLiteral("输入一条短消息"));
    messageLayout->addWidget(messageEdit_);

    auto *messageActionLayout = new QHBoxLayout();
    sendButton_ = UiHelpers::markPrimary(new QPushButton(QStringLiteral("发送"), this));
    sendButton_->setObjectName(QStringLiteral("sendMessageButton"));
    sendButton_->setEnabled(false);
    messageActionLayout->addWidget(sendButton_);

    markReadButton_ = UiHelpers::markSecondary(new QPushButton(QStringLiteral("标记已读"), this));
    markReadButton_->setObjectName(QStringLiteral("markReadButton"));
    markReadButton_->setEnabled(false);
    messageActionLayout->addWidget(markReadButton_);

    closeConversationButton_ = UiHelpers::markDanger(new QPushButton(QStringLiteral("关闭会话"), this));
    closeConversationButton_->setObjectName(QStringLiteral("closeConversationButton"));
    closeConversationButton_->setEnabled(false);
    messageActionLayout->addWidget(closeConversationButton_);
    messageActionLayout->addStretch();
    messageLayout->addLayout(messageActionLayout);
    rightLayout->addWidget(messageCard, 2);

    auto *contactCard = new QGroupBox(QStringLiteral("联系方式卡片"), rightPanel);
    auto *contactLayout = new QVBoxLayout(contactCard);

    auto *cardFormLayout = new QFormLayout();
    wechatEdit_ = new QLineEdit(this);
    wechatEdit_->setObjectName(QStringLiteral("wechatEdit"));
    wechatEdit_->setPlaceholderText(QStringLiteral("微信号"));
    cardFormLayout->addRow(QStringLiteral("微信"), wechatEdit_);

    phoneEdit_ = new QLineEdit(this);
    phoneEdit_->setObjectName(QStringLiteral("phoneEdit"));
    phoneEdit_->setPlaceholderText(QStringLiteral("手机号"));
    cardFormLayout->addRow(QStringLiteral("手机"), phoneEdit_);

    qqEdit_ = new QLineEdit(this);
    qqEdit_->setObjectName(QStringLiteral("qqEdit"));
    qqEdit_->setPlaceholderText(QStringLiteral("QQ号"));
    cardFormLayout->addRow(QStringLiteral("QQ"), qqEdit_);
    contactLayout->addLayout(cardFormLayout);

    saveContactCardButton_ = UiHelpers::markSecondary(new QPushButton(QStringLiteral("保存联系方式"), this));
    saveContactCardButton_->setObjectName(QStringLiteral("saveContactCardButton"));
    contactLayout->addWidget(saveContactCardButton_);

    contactCardStatusLabel_ = UiHelpers::createStatusLabel(this);
    contactCardStatusLabel_->setObjectName(QStringLiteral("contactCardStatusLabel"));
    contactLayout->addWidget(contactCardStatusLabel_);

    unlockStatusLabel_ = UiHelpers::createStatusLabel(this);
    unlockStatusLabel_->setObjectName(QStringLiteral("unlockStatusLabel"));
    contactLayout->addWidget(unlockStatusLabel_);

    auto *unlockActionLayout = new QHBoxLayout();
    confirmUnlockButton_ = UiHelpers::markPrimary(new QPushButton(QStringLiteral("确认交换联系方式"), this));
    confirmUnlockButton_->setObjectName(QStringLiteral("confirmUnlockButton"));
    confirmUnlockButton_->setEnabled(false);
    unlockActionLayout->addWidget(confirmUnlockButton_);

    viewPeerCardButton_ = UiHelpers::markSecondary(new QPushButton(QStringLiteral("查看对方联系方式"), this));
    viewPeerCardButton_->setObjectName(QStringLiteral("viewPeerCardButton"));
    viewPeerCardButton_->setEnabled(false);
    unlockActionLayout->addWidget(viewPeerCardButton_);
    contactLayout->addLayout(unlockActionLayout);

    peerCardLabel_ = new QLabel(this);
    peerCardLabel_->setObjectName(QStringLiteral("peerCardLabel"));
    peerCardLabel_->setWordWrap(true);
    contactLayout->addWidget(peerCardLabel_);
    rightLayout->addWidget(contactCard, 1);

    splitter->addWidget(listCard);
    splitter->addWidget(rightPanel);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 3);
    layout->addWidget(splitter, 1);

    statusLabel_ = UiHelpers::createStatusLabel(this);
    layout->addWidget(statusLabel_);

    connect(refreshButton_, &QPushButton::clicked, this, &ConversationsWidget::onRefreshConversations);
    connect(conversationListWidget_, &QListWidget::currentRowChanged, this, &ConversationsWidget::onConversationSelected);
    connect(sendButton_, &QPushButton::clicked, this, &ConversationsWidget::onSendMessage);
    connect(markReadButton_, &QPushButton::clicked, this, &ConversationsWidget::onMarkRead);
    connect(closeConversationButton_, &QPushButton::clicked, this, &ConversationsWidget::onCloseConversation);
    connect(saveContactCardButton_, &QPushButton::clicked, this, &ConversationsWidget::onSaveContactCard);
    connect(confirmUnlockButton_, &QPushButton::clicked, this, &ConversationsWidget::onConfirmUnlock);
    connect(viewPeerCardButton_, &QPushButton::clicked, this, &ConversationsWidget::onViewPeerContactCard);

    contactService_.getMyContactCard([this](const ContactCardResult &result) {
        if (result.success && result.hasCard) {
            wechatEdit_->setText(result.wechatId);
            phoneEdit_->setText(result.phoneNumber);
            qqEdit_->setText(result.qqNumber);
            contactCardStatusLabel_->setText(QStringLiteral("已加载联系方式"));
        }
    });
}

void ConversationsWidget::updateSendButtonState()
{
    sendButton_->setEnabled(currentConversationId_ > 0
        && currentConversationStatus_ == QStringLiteral("ACTIVE")
        && !currentConversationAwaitingPeerReply_);
}

void ConversationsWidget::updateMessageExchangeState(const QList<MessageItem> &messages)
{
    bool hasCurrentUserText = false;
    bool hasPeerText = false;

    for (const auto &msg : messages) {
        if (msg.messageType != QStringLiteral("USER_TEXT")) {
            continue;
        }
        if (!currentOtherParticipantId_.isEmpty() && msg.senderId == currentOtherParticipantId_) {
            hasPeerText = true;
        } else {
            hasCurrentUserText = true;
        }
    }

    currentConversationAwaitingPeerReply_ = hasCurrentUserText && !hasPeerText;
    updateSendButtonState();
}

void ConversationsWidget::updateUnlockUi(const ContactUnlockStatusResult &result)
{
    if (result.status == QStringLiteral("UNLOCKED")) {
        unlockStatusLabel_->setText(UiHelpers::contactUnlockStatusText(result.status));
        confirmUnlockButton_->setEnabled(false);
        viewPeerCardButton_->setEnabled(true);
    } else if (result.status == QStringLiteral("WAITING_FOR_PEER")) {
        unlockStatusLabel_->setText(UiHelpers::contactUnlockStatusText(result.status));
        confirmUnlockButton_->setEnabled(false);
        viewPeerCardButton_->setEnabled(false);
    } else {
        unlockStatusLabel_->setText(UiHelpers::contactUnlockStatusText(result.status));
        confirmUnlockButton_->setEnabled(currentConversationId_ > 0
            && currentConversationStatus_ == QStringLiteral("ACTIVE")
            && result.currentUserHasContactCard);
        viewPeerCardButton_->setEnabled(false);
    }
    peerCardLabel_->clear();
}

void ConversationsWidget::onRefreshConversations()
{
    UiHelpers::setButtonBusy(refreshButton_, true, QStringLiteral("加载中..."), QStringLiteral("刷新会话列表"));
    statusLabel_->setText(QStringLiteral("加载会话列表..."));
    contactService_.listConversations(0, 50, [this](const ConversationListResult &result) {
        UiHelpers::setButtonBusy(refreshButton_, false, QStringLiteral("加载中..."), QStringLiteral("刷新会话列表"));
        if (result.success) {
            conversations_ = result.items;
            conversationListWidget_->clear();
            for (const auto &item : conversations_) {
                QString display = QStringLiteral("%1\n%2 / %3")
                    .arg(item.relatedPostTitle,
                         item.otherParticipantDisplayName,
                         UiHelpers::statusDisplayName(item.status));
                if (item.unreadCount > 0) {
                    display += QStringLiteral(" (%1未读)").arg(item.unreadCount);
                }
                if (!item.lastMessagePreview.isEmpty()) {
                    display += QStringLiteral(" | ") + item.lastMessagePreview.left(30);
                }
                conversationListWidget_->addItem(display);
            }
            statusLabel_->setText(conversations_.isEmpty()
                ? UiHelpers::emptyStateText(QStringLiteral("conversations"))
                : QStringLiteral("共 %1 个会话").arg(conversations_.size()));
        } else {
            statusLabel_->setText(QStringLiteral("加载失败: %1 - %2").arg(result.errorCode).arg(result.errorMessage));
        }
    });
}

void ConversationsWidget::onConversationSelected()
{
    int idx = conversationListWidget_->currentRow();
    if (idx < 0 || idx >= conversations_.size()) {
        currentConversationId_ = 0;
        currentConversationStatus_.clear();
        currentOtherParticipantId_.clear();
        currentConversationAwaitingPeerReply_ = false;
        messageListWidget_->clear();
        sendButton_->setEnabled(false);
        markReadButton_->setEnabled(false);
        closeConversationButton_->setEnabled(false);
        conversationStatusLabel_->clear();
        unlockStatusLabel_->clear();
        confirmUnlockButton_->setEnabled(false);
        viewPeerCardButton_->setEnabled(false);
        peerCardLabel_->clear();
        return;
    }

    const auto &conv = conversations_[idx];
    currentConversationId_ = conv.conversationId;
    currentConversationStatus_ = conv.status;
    currentOtherParticipantId_ = conv.otherParticipantId;
    currentConversationAwaitingPeerReply_ = false;

    QString statusText = QStringLiteral("会话 #%1 · %2 · %3<br>评价用会话ID：%1；被评价者ID：%4")
        .arg(conv.conversationId)
        .arg(conv.otherParticipantDisplayName)
        .arg(UiHelpers::statusDisplayName(conv.status))
        .arg(conv.otherParticipantId);
    if (conv.unreadCount > 0) {
        statusText += QStringLiteral(" %1未读").arg(conv.unreadCount);
    }
    conversationStatusLabel_->setText(statusText);

    updateSendButtonState();
    markReadButton_->setEnabled(currentConversationId_ > 0 && conv.unreadCount > 0);
    closeConversationButton_->setEnabled(currentConversationId_ > 0 && currentConversationStatus_ == QStringLiteral("ACTIVE"));

    statusLabel_->setText(QStringLiteral("加载消息..."));
    contactService_.queryMessages(currentConversationId_, 0, 50, [this](const MessageListResult &result) {
        if (result.success) {
            messageListWidget_->clear();
            for (const auto &msg : result.items) {
                QString display = QStringLiteral("%1\n%2：%3")
                    .arg(msg.createdAt.left(19), msg.senderId.left(8), msg.content.left(100));
                messageListWidget_->addItem(display);
            }
            updateMessageExchangeState(result.items);
            statusLabel_->setText(result.items.isEmpty()
                ? UiHelpers::emptyStateText(QStringLiteral("messages"))
                : QStringLiteral("共 %1 条消息").arg(result.items.size()));
            if (currentConversationAwaitingPeerReply_) {
                statusLabel_->setText(QStringLiteral("已发送初始邀约，请等待对方回复后再继续发送消息"));
            }
        } else {
            statusLabel_->setText(QStringLiteral("消息加载失败: %1 - %2").arg(result.errorCode).arg(result.errorMessage));
        }
    });

    onRefreshUnlockStatus();
}

void ConversationsWidget::onSendMessage()
{
    if (currentConversationId_ <= 0) return;

    QString message = messageEdit_->text().trimmed();
    if (message.isEmpty()) {
        statusLabel_->setText(QStringLiteral("消息不能为空"));
        return;
    }

    UiHelpers::setButtonBusy(sendButton_, true, QStringLiteral("发送中..."), QStringLiteral("发送"));
    statusLabel_->setText(QStringLiteral("发送中..."));
    contactService_.sendMessage(currentConversationId_, message, [this](const SendMessageResult &result) {
        if (result.success) {
            messageEdit_->clear();
            UiHelpers::setButtonBusy(sendButton_, false, QStringLiteral("发送中..."), QStringLiteral("发送"));
            updateSendButtonState();
            onConversationSelected();
        } else {
            UiHelpers::setButtonBusy(sendButton_, false, QStringLiteral("发送中..."), QStringLiteral("发送"));
            if (result.errorCode == QStringLiteral("CONVERSATION_CLOSED")) {
                currentConversationStatus_ = QStringLiteral("CLOSED");
                updateSendButtonState();
                closeConversationButton_->setEnabled(false);
            } else if (result.errorCode == QStringLiteral("CONTACT_REPLY_REQUIRED")) {
                currentConversationAwaitingPeerReply_ = true;
                updateSendButtonState();
                statusLabel_->setText(QStringLiteral("发送失败: 请等待对方回复后再继续发送消息"));
                return;
            }
            statusLabel_->setText(QStringLiteral("发送失败: %1 - %2").arg(result.errorCode).arg(result.errorMessage));
        }
    });
}

void ConversationsWidget::onMarkRead()
{
    if (currentConversationId_ <= 0) return;

    UiHelpers::setButtonBusy(markReadButton_, true, QStringLiteral("标记中..."), QStringLiteral("标记已读"));
    statusLabel_->setText(QStringLiteral("标记已读..."));
    contactService_.markConversationRead(currentConversationId_, [this](const MarkReadResult &result) {
        UiHelpers::setButtonBusy(markReadButton_, false, QStringLiteral("标记中..."), QStringLiteral("标记已读"));
        if (result.success) {
            statusLabel_->setText(QStringLiteral("已标记已读"));
            onRefreshConversations();
        } else {
            statusLabel_->setText(QStringLiteral("标记已读失败: %1 - %2").arg(result.errorCode).arg(result.errorMessage));
        }
    });
}

void ConversationsWidget::onCloseConversation()
{
    if (currentConversationId_ <= 0) return;

    UiHelpers::setButtonBusy(closeConversationButton_, true, QStringLiteral("关闭中..."), QStringLiteral("关闭会话"));
    statusLabel_->setText(QStringLiteral("关闭会话..."));
    contactService_.closeConversation(currentConversationId_, [this](const CloseConversationResult &result) {
        UiHelpers::setButtonBusy(closeConversationButton_, false, QStringLiteral("关闭中..."), QStringLiteral("关闭会话"));
        if (result.success) {
            statusLabel_->setText(QStringLiteral("会话已关闭"));
            onRefreshConversations();
        } else {
            statusLabel_->setText(QStringLiteral("关闭失败: %1 - %2").arg(result.errorCode).arg(result.errorMessage));
        }
    });
}

void ConversationsWidget::onSaveContactCard()
{
    UiHelpers::setButtonBusy(saveContactCardButton_, true, QStringLiteral("保存中..."), QStringLiteral("保存联系方式"));
    contactCardStatusLabel_->setText(QStringLiteral("保存中..."));
    contactService_.upsertMyContactCard(
        wechatEdit_->text().trimmed(),
        phoneEdit_->text().trimmed(),
        qqEdit_->text().trimmed(),
        [this](const ContactCardResult &result) {
            UiHelpers::setButtonBusy(saveContactCardButton_, false, QStringLiteral("保存中..."), QStringLiteral("保存联系方式"));
            if (result.success) {
                contactCardStatusLabel_->setText(QStringLiteral("联系方式已保存"));
            } else {
                contactCardStatusLabel_->setText(QStringLiteral("保存失败: %1 - %2").arg(result.errorCode).arg(result.errorMessage));
            }
        });
}

void ConversationsWidget::onRefreshUnlockStatus()
{
    if (currentConversationId_ <= 0) {
        unlockStatusLabel_->clear();
        confirmUnlockButton_->setEnabled(false);
        viewPeerCardButton_->setEnabled(false);
        return;
    }

    contactService_.getContactUnlockStatus(currentConversationId_, [this](const ContactUnlockStatusResult &result) {
        if (result.success) {
            updateUnlockUi(result);
        } else {
            unlockStatusLabel_->setText(QStringLiteral("查询解锁状态失败: %1").arg(result.errorCode));
            confirmUnlockButton_->setEnabled(false);
            viewPeerCardButton_->setEnabled(false);
        }
    });
}

void ConversationsWidget::onConfirmUnlock()
{
    if (currentConversationId_ <= 0) return;

    UiHelpers::setButtonBusy(confirmUnlockButton_, true, QStringLiteral("确认中..."), QStringLiteral("确认交换联系方式"));
    statusLabel_->setText(QStringLiteral("确认交换..."));
    contactService_.confirmContactUnlock(currentConversationId_, [this](const ContactUnlockStatusResult &result) {
        UiHelpers::setButtonBusy(confirmUnlockButton_, false, QStringLiteral("确认中..."), QStringLiteral("确认交换联系方式"));
        if (result.success) {
            updateUnlockUi(result);
            if (result.status == QStringLiteral("UNLOCKED")) {
                statusLabel_->setText(QStringLiteral("联系方式已解锁，可查看对方卡片"));
            } else if (result.status == QStringLiteral("WAITING_FOR_PEER")) {
                statusLabel_->setText(QStringLiteral("已确认，等待对方确认"));
            }
        } else {
            statusLabel_->setText(QStringLiteral("确认失败: %1 - %2").arg(result.errorCode).arg(result.errorMessage));
            onRefreshUnlockStatus();
        }
    });
}

void ConversationsWidget::onViewPeerContactCard()
{
    if (currentConversationId_ <= 0) return;

    UiHelpers::setButtonBusy(viewPeerCardButton_, true, QStringLiteral("获取中..."), QStringLiteral("查看对方联系方式"));
    statusLabel_->setText(QStringLiteral("获取对方联系方式..."));
    contactService_.getPeerContactCard(currentConversationId_, [this](const PeerContactCardResult &result) {
        if (result.success) {
            QString info = QStringLiteral("<b>对方联系方式</b><br>微信：%1<br>手机：%2<br>QQ：%3")
                .arg((result.wechatId.isEmpty() ? QStringLiteral("(未填)") : result.wechatId).toHtmlEscaped())
                .arg((result.phoneNumber.isEmpty() ? QStringLiteral("(未填)") : result.phoneNumber).toHtmlEscaped())
                .arg((result.qqNumber.isEmpty() ? QStringLiteral("(未填)") : result.qqNumber).toHtmlEscaped());
            peerCardLabel_->setText(info);
            statusLabel_->setText(QStringLiteral("已获取对方联系方式"));
            UiHelpers::setButtonBusy(viewPeerCardButton_, false, QStringLiteral("获取中..."), QStringLiteral("查看对方联系方式"));
        } else {
            peerCardLabel_->clear();
            statusLabel_->setText(QStringLiteral("获取失败: %1 - %2").arg(result.errorCode).arg(result.errorMessage));
            UiHelpers::setButtonBusy(viewPeerCardButton_, false, QStringLiteral("获取中..."), QStringLiteral("查看对方联系方式"));
        }
    });
}
