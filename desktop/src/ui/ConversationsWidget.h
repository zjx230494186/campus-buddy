#pragma once

#include <QLabel>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QWidget>

#include "api/ContactConversationApiService.h"
#include "domain/ContactConversationModels.h"

class ConversationsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ConversationsWidget(ContactConversationApiService &contactService, QWidget *parent = nullptr);

public slots:
    void onRefreshConversations();

private slots:
    void onConversationSelected();
    void onSendMessage();
    void onMarkRead();
    void onCloseConversation();
    void onSaveContactCard();
    void onRefreshUnlockStatus();
    void onConfirmUnlock();
    void onViewPeerContactCard();

private:
    void updateSendButtonState();
    void updateMessageExchangeState(const QList<MessageItem> &messages);
    void updateUnlockUi(const ContactUnlockStatusResult &result);

    ContactConversationApiService &contactService_;

    QListWidget *conversationListWidget_;
    QLabel *conversationStatusLabel_;
    QListWidget *messageListWidget_;
    QLineEdit *messageEdit_;
    QPushButton *sendButton_;
    QPushButton *refreshButton_;
    QPushButton *markReadButton_;
    QPushButton *closeConversationButton_;
    QLabel *statusLabel_;

    QLineEdit *wechatEdit_;
    QLineEdit *phoneEdit_;
    QLineEdit *qqEdit_;
    QPushButton *saveContactCardButton_;
    QLabel *contactCardStatusLabel_;

    QLabel *unlockStatusLabel_;
    QPushButton *confirmUnlockButton_;
    QPushButton *viewPeerCardButton_;
    QLabel *peerCardLabel_;

    QList<ConversationListItem> conversations_;
    long long currentConversationId_ = 0;
    QString currentConversationStatus_;
    QString currentOtherParticipantId_;
    bool currentConversationAwaitingPeerReply_ = false;
};
