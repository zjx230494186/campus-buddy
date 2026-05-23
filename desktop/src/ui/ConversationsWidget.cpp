#include "ui/ConversationsWidget.h"

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

    statusLabel_ = new QLabel(this);
    layout->addWidget(statusLabel_);

    connect(refreshButton_, &QPushButton::clicked, this, &ConversationsWidget::onRefreshConversations);
    connect(conversationListWidget_, &QListWidget::currentRowChanged, this, &ConversationsWidget::onConversationSelected);
    connect(sendButton_, &QPushButton::clicked, this, &ConversationsWidget::onSendMessage);
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
        messageListWidget_->clear();
        sendButton_->setEnabled(false);
        conversationStatusLabel_->clear();
        return;
    }

    const auto &conv = conversations_[idx];
    currentConversationId_ = conv.conversationId;
    conversationStatusLabel_->setText(
        QStringLiteral("会话 %1: %2 [%3]").arg(conv.conversationId).arg(conv.otherParticipantDisplayName).arg(conv.status));
    sendButton_->setEnabled(true);

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
            sendButton_->setEnabled(true);
            onConversationSelected();
        } else {
            statusLabel_->setText(QStringLiteral("发送失败: %1 - %2").arg(result.errorCode).arg(result.errorMessage));
            sendButton_->setEnabled(true);
        }
    });
}
