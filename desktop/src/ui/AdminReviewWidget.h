#pragma once

#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QTextEdit>
#include <QWidget>

#include "api/AdminReviewApiService.h"

class AdminReviewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AdminReviewWidget(AdminReviewApiService &adminService, QWidget *parent = nullptr);

private slots:
    void onRefreshPostQueue();
    void onPostQueueItemClicked(QListWidgetItem *item);
    void onApprovePost();
    void onRejectPost();
    void onRefreshIdentityQueue();
    void onApproveIdentity();
    void onRejectIdentity();

private:
    AdminReviewApiService &adminService_;

    QTabWidget *innerTab_;

    QListWidget *postQueueList_;
    QPushButton *refreshPostQueueButton_;
    QLabel *postDetailLabel_;
    QPushButton *approvePostButton_;
    QPushButton *rejectPostButton_;
    QLineEdit *rejectPostReasonEdit_;

    QListWidget *identityQueueList_;
    QPushButton *refreshIdentityQueueButton_;
    QLabel *identityDetailLabel_;
    QPushButton *approveIdentityButton_;
    QPushButton *rejectIdentityButton_;
    QLineEdit *rejectIdentityReasonEdit_;

    QLabel *statusLabel_;

    QString selectedPostId_;
    long long selectedSubmissionId_ = 0;
};
