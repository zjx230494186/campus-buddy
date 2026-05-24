#include "ui/ConversationsWidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>

ConversationsWidget::ConversationsWidget(ContactConversationApiService &contactService, QWidget *parent)
    : QWidget(parent),
      contactService_(contactService)
{
    auto *layout = new QVBoxLayout(this);

    auto *header = new QLabel(QStringLiteral("会话"), this);
    header->setAlignment(Qt::AlignCenter);
    QFont f = header->font();
    f.setPointSize(14);
    f.setBold(true);
    header->setFont(f);
    layout->addWidget(header);

    refreshButton_ = new QPushButton(QStringLiteral("刷新会话列表"), this);
    refreshButton_->setObjectName(QStringLiteral("refreshConversationsButton"));
    layout->addWidget(refreshButton_);

    conversationListWidget_ = new QListWidget(this);
    conversationListWidget_->setObjectName(QStringLiteral("conversationListWidget"));
    conversationListWidget_->setMaximumHeight(200);
    layout->addWidget(conversationListWidget_);

    conversationStatusLabel_ = new QLabel(this);
    conversationStatusLabel_->setObjectName(QStringLiteral("conversationStatusLabel"));
    layout->addWidget(conversationStatusLabel_);

    auto *msgHeader = new QLabel(QStringLiteral("消息"), this);
    layout->addWidget(msgHeader);

    messageListWidget_ = new QListWidget(this);
    messageListWidget_->setObjectName(QStringLiteral("messageListWidget"));
    layout->addWidget(messageListWidget_);

    messageEdit_ = new QLineEdit(this);
    messageEdit_->setObjectName(QStringLiteral("messageEdit"));
    messageEdit_->setPlaceholderText(QStringLiteral("输入消息"));
    layout->addWidget(messageEdit_);

    sendButton_ = new QPushButton(QStringLiteral("发送"), this);
    sendButton_->setObjectName(QStringLiteral("sendMessageButton"));
    sendButton_->setEnabled(false);
    layout->addWidget(sendButton_);

    auto *actionLayout = new QHBoxLayout();
    markReadButton_ = new QPushButton(QStringLiteral("标记已读"), this);
    markReadButton_->setObjectName(QStringLiteral("markReadButton"));
    markReadButton_->setEnabled(false);
    actionLayout->addWidget(markReadButton_);

    closeConversationButton_ = new QPushButton(QStringLiteral("关闭会话"), this);
    closeConversationButton_->setObjectName(QStringLiteral("closeConversationButton"));
    closeConversationButton_->setEnabled(false);
    actionLayout->addWidget(closeConversationButton_);

    layout->addLayout(actionLayout);

    auto *cardHeader = new QLabel(QStringLiteral("我的联系方式"), this);
    f.setPointSize(11);
    f.setBold(true);
    cardHeader->setFont(f);
    layout->addWidget(cardHeader);

    auto *cardFormLayout = new QHBoxLayout();
    cardFormLayout->addWidget(new QLabel(QStringLiteral("微信:"), this));
    wechatEdit_ = new QLineEdit(this);
    wechatEdit_->setObjectName(QStringLiteral("wechatEdit"));
    wechatEdit_->setPlaceholderText(QStringLiteral("微信号"));
    cardFormLayout->addWidget(wechatEdit_);

    cardFormLayout->addWidget(new QLabel(QStringLiteral("手机:"), this));
    phoneEdit_ = new QLineEdit(this);
    phoneEdit_->setObjectName(QStringLiteral("phoneEdit"));
    phoneEdit_->setPlaceholderText(QStringLiteral("手机号"));
    cardFormLayout->addWidget(phoneEdit_);

    cardFormLayout->addWidget(new QLabel(QStringLiteral("QQ:"), this));
    qqEdit_ = new QLineEdit(this);
    qqEdit_->setObjectName(QStringLiteral("qqEdit"));
    qqEdit_->setPlaceholderText(QStringLiteral("QQ号"));
    cardFormLayout->addWidget(qqEdit_);
    layout->addLayout(cardFormLayout);

    saveContactCardButton_ = new QPushButton(QStringLiteral("保存联系方式"), this);
    saveContactCardButton_->setObjectName(QStringLiteral("saveContactCardButton"));
    layout->addWidget(saveContactCardButton_);

    contactCardStatusLabel_ = new QLabel(this);
    contactCardStatusLabel_->setObjectName(QStringLiteral("contactCardStatusLabel"));
    layout->addWidget(contactCardStatusLabel_);

    auto *unlockHeader = new QLabel(QStringLiteral("联系方式交换"), this);
    unlockHeader->setFont(f);
    layout->addWidget(unlockHeader);

    unlockStatusLabel_ = new QLabel(this);
    unlockStatusLabel_->setObjectName(QStringLiteral("unlockStatusLabel"));
    layout->addWidget(unlockStatusLabel_);

    auto *unlockActionLayout = new QHBoxLayout();
    confirmUnlockButton_ = new QPushButton(QStringLiteral("确认交换联系方式"), this);
    confirmUnlockButton_->setObjectName(QStringLiteral("confirmUnlockButton"));
    confirmUnlockButton_->setEnabled(false);
    unlockActionLayout->addWidget(confirmUnlockButton_);

    viewPeerCardButton_ = new QPushButton(QStringLiteral("查看对方联系方式"), this);
    viewPeerCardButton_->setObjectName(QStringLiteral("viewPeerCardButton"));
    viewPeerCardButton_->setEnabled(false);
    unlockActionLayout->addWidget(viewPeerCardButton_);
    layout->addLayout(unlockActionLayout);

    peerCardLabel_ = new QLabel(this);
    peerCardLabel_->setObjectName(QStringLiteral("peerCardLabel"));
    layout->addWidget(peerCardLabel_);

    statusLabel_ = new QLabel(this);
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
    sendButton_->setEnabled(currentConversationId_ > 0 && currentConversationStatus_ == QStringLiteral("ACTIVE"));
}

void ConversationsWidget::updateUnlockUi(const ContactUnlockStatusResult &result)
{
    if (result.status == QStringLiteral("UNLOCKED")) {
        unlockStatusLabel_->setText(QStringLiteral("状态: 已解锁 - 双方已确认交换"));
        confirmUnlockButton_->setEnabled(false);
        viewPeerCardButton_->setEnabled(true);
    } else if (result.status == QStringLiteral("WAITING_FOR_PEER")) {
        unlockStatusLabel_->setText(QStringLiteral("状态: 等待对方确认"));
        confirmUnlockButton_->setEnabled(false);
        viewPeerCardButton_->setEnabled(false);
    } else {
        unlockStatusLabel_->setText(QStringLiteral("状态: 未解锁"));
        confirmUnlockButton_->setEnabled(currentConversationId_ > 0
            && currentConversationStatus_ == QStringLiteral("ACTIVE")
            && result.currentUserHasContactCard);
        viewPeerCardButton_->setEnabled(false);
    }
    peerCardLabel_->clear();
}

void ConversationsWidget::onRefreshConversations()
{
    statusLabel_->setText(QStringLiteral("加载会话列表..."));
    contactService_.listConversations(0, 50, [this](const ConversationListResult &result) {
        if (result.success) {
            conversations_ = result.items;
            conversationListWidget_->clear();
            for (const auto &item : conversations_) {
                QString display = QStringLiteral("[%1] %2 - %3").arg(item.status, item.otherParticipantDisplayName, item.relatedPostTitle);
                if (item.unreadCount > 0) {
                    display += QStringLiteral(" (%1未读)").arg(item.unreadCount);
                }
                if (!item.lastMessagePreview.isEmpty()) {
                    display += QStringLiteral(" | ") + item.lastMessagePreview.left(30);
                }
                conversationListWidget_->addItem(display);
            }
            statusLabel_->setText(QStringLiteral("共 %1 个会话").arg(conversations_.size()));
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

    QString statusText = QStringLiteral("会话 %1: %2 [%3]").arg(conv.conversationId).arg(conv.otherParticipantDisplayName).arg(conv.status);
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
                QString display = QStringLiteral("[%1] %2: %3").arg(msg.createdAt.left(19), msg.senderId.left(8), msg.content.left(100));
                messageListWidget_->addItem(display);
            }
            statusLabel_->setText(QStringLiteral("共 %1 条消息").arg(result.items.size()));
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

    sendButton_->setEnabled(false);
    statusLabel_->setText(QStringLiteral("发送中..."));
    contactService_.sendMessage(currentConversationId_, message, [this](const SendMessageResult &result) {
        if (result.success) {
            messageEdit_->clear();
            updateSendButtonState();
            onConversationSelected();
        } else {
            if (result.errorCode == QStringLiteral("CONVERSATION_CLOSED")) {
                currentConversationStatus_ = QStringLiteral("CLOSED");
                updateSendButtonState();
                closeConversationButton_->setEnabled(false);
            }
            statusLabel_->setText(QStringLiteral("发送失败: %1 - %2").arg(result.errorCode).arg(result.errorMessage));
        }
    });
}

void ConversationsWidget::onMarkRead()
{
    if (currentConversationId_ <= 0) return;

    statusLabel_->setText(QStringLiteral("标记已读..."));
    contactService_.markConversationRead(currentConversationId_, [this](const MarkReadResult &result) {
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

    statusLabel_->setText(QStringLiteral("关闭会话..."));
    contactService_.closeConversation(currentConversationId_, [this](const CloseConversationResult &result) {
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
    contactCardStatusLabel_->setText(QStringLiteral("保存中..."));
    contactService_.upsertMyContactCard(
        wechatEdit_->text().trimmed(),
        phoneEdit_->text().trimmed(),
        qqEdit_->text().trimmed(),
        [this](const ContactCardResult &result) {
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

    confirmUnlockButton_->setEnabled(false);
    statusLabel_->setText(QStringLiteral("确认交换..."));
    contactService_.confirmContactUnlock(currentConversationId_, [this](const ContactUnlockStatusResult &result) {
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

    viewPeerCardButton_->setEnabled(false);
    statusLabel_->setText(QStringLiteral("获取对方联系方式..."));
    contactService_.getPeerContactCard(currentConversationId_, [this](const PeerContactCardResult &result) {
        if (result.success) {
            QString info = QStringLiteral("微信: %1  手机: %2  QQ: %3")
                .arg(result.wechatId.isEmpty() ? QStringLiteral("(未填)") : result.wechatId)
                .arg(result.phoneNumber.isEmpty() ? QStringLiteral("(未填)") : result.phoneNumber)
                .arg(result.qqNumber.isEmpty() ? QStringLiteral("(未填)") : result.qqNumber);
            peerCardLabel_->setText(info);
            statusLabel_->setText(QStringLiteral("已获取对方联系方式"));
            viewPeerCardButton_->setEnabled(true);
        } else {
            peerCardLabel_->clear();
            statusLabel_->setText(QStringLiteral("获取失败: %1 - %2").arg(result.errorCode).arg(result.errorMessage));
            viewPeerCardButton_->setEnabled(true);
        }
    });
}
