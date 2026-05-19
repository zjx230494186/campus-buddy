#pragma once

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QWidget>

#include "auth/AuthApiService.h"

class IdentityVerificationWidget : public QWidget
{
    Q_OBJECT

public:
    explicit IdentityVerificationWidget(AuthApiService &authService, QWidget *parent = nullptr);

    void setStatus(const QString &authenticationStatus, const QString &rejectReason);

signals:
    void submissionSuccess();

private slots:
    void onSelectFileClicked();
    void onUploadFileClicked();
    void onSubmitClicked();

private:
    AuthApiService &authService_;
    QString attachmentId_;
    QString selectedFileName_;
    QByteArray selectedFileData_;
    QString selectedContentType_;

    QLineEdit *realNameEdit_;
    QLineEdit *studentNumberEdit_;
    QLineEdit *collegeEdit_;
    QLineEdit *majorEdit_;
    QLineEdit *gradeEdit_;
    QPushButton *selectFileButton_;
    QLabel *fileLabel_;
    QPushButton *uploadFileButton_;
    QPushButton *submitButton_;
    QLabel *statusLabel_;
    QLabel *rejectReasonLabel_;
};
