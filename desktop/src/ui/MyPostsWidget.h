#pragma once

#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QWidget>

#include "api/MyPartnerPostApiService.h"

class MyPostsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MyPostsWidget(MyPartnerPostApiService &myPostService, QWidget *parent = nullptr);

signals:
    void editPostRequested(const QString &postId, const MyPostItem &item);

private slots:
    void onRefresh();
    void onItemSelected();
    void onWithdrawReview();
    void onUnpublish();

private:
    void updateActionButtons();

    MyPartnerPostApiService &myPostService_;
    QListWidget *listWidget_;
    QLabel *statusLabel_;
    QLabel *detailLabel_;
    QPushButton *refreshButton_;
    QPushButton *editButton_;
    QPushButton *withdrawButton_;
    QPushButton *unpublishButton_;

    QList<MyPostItem> items_;
    int selectedIndex_ = -1;
};
