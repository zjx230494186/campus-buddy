#pragma once

#include <QLabel>
#include <QPushButton>
#include <QTabWidget>
#include <QWidget>

#include "auth/AuthApiService.h"
#include "api/MyPartnerPostApiService.h"
#include "api/PartnerPostApiService.h"
#include "api/ContactConversationApiService.h"
#include "ui/IdentityVerificationWidget.h"
#include "ui/PostEditorWidget.h"
#include "ui/MyPostsWidget.h"
#include "ui/PlazaWidget.h"

class HomePageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HomePageWidget(AuthApiService &authService,
                            MyPartnerPostApiService &myPostService,
                            PartnerPostApiService &plazaService,
                            ContactConversationApiService &contactService,
                            QWidget *parent = nullptr);

signals:
    void logout();

private slots:
    void onLogoutClicked();
    void onCheckVerificationStatus();
    void onEditPostRequested(const QString &postId, const MyPostItem &item);

private:
    AuthApiService &authService_;

    QTabWidget *tabWidget_;
    IdentityVerificationWidget *verificationWidget_;
    PostEditorWidget *postEditorWidget_;
    MyPostsWidget *myPostsWidget_;
    PlazaWidget *plazaWidget_;

    QPushButton *logoutButton_;
    QPushButton *checkStatusButton_;
    QLabel *statusLabel_;
    QLabel *verificationStatusLabel_;
};
