#pragma once

#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QWidget>

#include "api/ReviewCreditApiService.h"

class ReviewCreditWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ReviewCreditWidget(ReviewCreditApiService &reviewService, QWidget *parent = nullptr);

private slots:
    void onRefreshCreditSummary();
    void onSubmitReview();
    void onUpdateReview();
    void onRefreshGivenReviews();
    void onRefreshReceivedReviews();

private:
    ReviewCreditApiService &reviewService_;

    QLabel *creditSummaryLabel_;
    QPushButton *refreshCreditButton_;

    QLineEdit *convIdEdit_;
    QLineEdit *revieweeIdEdit_;
    QSpinBox *ratingSpin_;
    QLineEdit *reviewTagsEdit_;
    QPushButton *submitReviewButton_;

    QLineEdit *reviewIdEdit_;
    QSpinBox *updateRatingSpin_;
    QLineEdit *updateTagsEdit_;
    QPushButton *updateReviewButton_;

    QListWidget *givenListWidget_;
    QPushButton *refreshGivenButton_;
    QListWidget *receivedListWidget_;
    QPushButton *refreshReceivedButton_;

    QLabel *statusLabel_;
};
