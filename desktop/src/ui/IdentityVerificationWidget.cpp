#include "ui/IdentityVerificationWidget.h"
#include "ui/UiHelpers.h"

#include <QByteArray>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>

IdentityVerificationWidget::IdentityVerificationWidget(AuthApiService &authService, QWidget *parent)
    : QWidget(parent),
      authService_(authService)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(12);

    layout->addWidget(UiHelpers::createPageHeader(
        QStringLiteral("校园身份认证"),
        QStringLiteral("提交基本学籍信息和认证材料，管理员审核通过后才能发布需求。"),
        this));

    auto *profileGroup = new QGroupBox(QStringLiteral("基本信息"), this);
    auto *formLayout = new QFormLayout(profileGroup);
    formLayout->setLabelAlignment(Qt::AlignRight);
    realNameEdit_ = new QLineEdit(this);
    realNameEdit_->setPlaceholderText(QStringLiteral("真实姓名"));
    formLayout->addRow(QStringLiteral("姓名"), realNameEdit_);

    studentNumberEdit_ = new QLineEdit(this);
    studentNumberEdit_->setPlaceholderText(QStringLiteral("学号"));
    formLayout->addRow(QStringLiteral("学号"), studentNumberEdit_);

    collegeEdit_ = new QLineEdit(this);
    collegeEdit_->setPlaceholderText(QStringLiteral("学院"));
    formLayout->addRow(QStringLiteral("学院"), collegeEdit_);

    majorEdit_ = new QLineEdit(this);
    majorEdit_->setPlaceholderText(QStringLiteral("专业"));
    formLayout->addRow(QStringLiteral("专业"), majorEdit_);

    gradeEdit_ = new QLineEdit(this);
    gradeEdit_->setPlaceholderText(QStringLiteral("年级"));
    formLayout->addRow(QStringLiteral("年级"), gradeEdit_);

    layout->addWidget(profileGroup);

    auto *materialGroup = new QGroupBox(QStringLiteral("认证材料"), this);
    auto *materialLayout = new QVBoxLayout(materialGroup);

    auto *fileActionLayout = new QHBoxLayout();
    selectFileButton_ = UiHelpers::markSecondary(new QPushButton(QStringLiteral("选择材料"), this));
    fileActionLayout->addWidget(selectFileButton_);

    fileLabel_ = new QLabel(QStringLiteral("未选择文件"), this);
    fileLabel_->setProperty("muted", true);
    fileActionLayout->addWidget(fileLabel_, 1);
    materialLayout->addLayout(fileActionLayout);

    auto *submitActionLayout = new QHBoxLayout();
    uploadFileButton_ = UiHelpers::markSecondary(new QPushButton(QStringLiteral("上传材料"), this));
    uploadFileButton_->setEnabled(false);
    submitActionLayout->addWidget(uploadFileButton_);

    submitButton_ = UiHelpers::markPrimary(new QPushButton(QStringLiteral("提交认证"), this));
    submitButton_->setEnabled(false);
    submitActionLayout->addWidget(submitButton_);
    submitActionLayout->addStretch();
    materialLayout->addLayout(submitActionLayout);
    layout->addWidget(materialGroup);

    rejectReasonLabel_ = UiHelpers::createStatusLabel(this);
    layout->addWidget(rejectReasonLabel_);

    statusLabel_ = UiHelpers::createStatusLabel(this);
    layout->addWidget(statusLabel_);
    layout->addStretch();

    connect(selectFileButton_, &QPushButton::clicked, this, &IdentityVerificationWidget::onSelectFileClicked);
    connect(uploadFileButton_, &QPushButton::clicked, this, &IdentityVerificationWidget::onUploadFileClicked);
    connect(submitButton_, &QPushButton::clicked, this, &IdentityVerificationWidget::onSubmitClicked);
}

void IdentityVerificationWidget::setStatus(const QString &authenticationStatus, const QString &rejectReason)
{
    if (authenticationStatus == QStringLiteral("PENDING_REVIEW")) {
        statusLabel_->setText(QStringLiteral("认证审核中，请等待"));
        setEnabled(false);
    } else if (authenticationStatus == QStringLiteral("VERIFIED")) {
        statusLabel_->setText(QStringLiteral("已通过认证"));
        setEnabled(false);
    } else {
        setEnabled(true);
        if (authenticationStatus == QStringLiteral("REJECTED") && !rejectReason.isEmpty()) {
            rejectReasonLabel_->setText(QStringLiteral("驳回原因: %1").arg(rejectReason));
        } else {
            rejectReasonLabel_->clear();
        }
        statusLabel_->clear();
    }
}

void IdentityVerificationWidget::onSelectFileClicked()
{
    const QString filePath = QFileDialog::getOpenFileName(this,
        QStringLiteral("选择认证材料"),
        QString(),
        QStringLiteral("图片 (*.jpg *.jpeg *.png);;PDF (*.pdf)"));

    if (filePath.isEmpty()) {
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        statusLabel_->setText(QStringLiteral("无法打开文件"));
        return;
    }

    selectedFileData_ = file.readAll();
    file.close();

    QFileInfo fi(filePath);
    selectedFileName_ = fi.fileName();

    const QString suffix = fi.suffix().toLower();
    if (suffix == QStringLiteral("jpg") || suffix == QStringLiteral("jpeg")) {
        selectedContentType_ = QStringLiteral("image/jpeg");
    } else if (suffix == QStringLiteral("png")) {
        selectedContentType_ = QStringLiteral("image/png");
    } else if (suffix == QStringLiteral("pdf")) {
        selectedContentType_ = QStringLiteral("application/pdf");
    } else {
        selectedContentType_ = QStringLiteral("application/octet-stream");
    }

    fileLabel_->setText(QStringLiteral("%1 (%2 KB)").arg(selectedFileName_).arg(selectedFileData_.size() / 1024));
    uploadFileButton_->setEnabled(true);
    attachmentId_.clear();
    submitButton_->setEnabled(false);
}

void IdentityVerificationWidget::onUploadFileClicked()
{
    if (selectedFileData_.isEmpty()) {
        statusLabel_->setText(QStringLiteral("请先选择文件"));
        return;
    }

    uploadFileButton_->setEnabled(false);
    statusLabel_->setText(QStringLiteral("上传中..."));

    authService_.uploadIdentityMaterial(selectedFileData_, selectedFileName_, selectedContentType_, [this](const AuthResult &result) {
        uploadFileButton_->setEnabled(true);
        if (result.success) {
            attachmentId_ = result.attachmentId;
            statusLabel_->setText(QStringLiteral("上传成功"));
            submitButton_->setEnabled(true);
        } else {
            attachmentId_.clear();
            statusLabel_->setText(result.errorMessage.isEmpty() ? QStringLiteral("上传失败") : result.errorMessage);
            submitButton_->setEnabled(false);
        }
    });
}

void IdentityVerificationWidget::onSubmitClicked()
{
    const QString realName = realNameEdit_->text().trimmed();
    const QString studentNumber = studentNumberEdit_->text().trimmed();
    const QString college = collegeEdit_->text().trimmed();
    const QString major = majorEdit_->text().trimmed();
    const QString grade = gradeEdit_->text().trimmed();

    if (realName.isEmpty() || studentNumber.isEmpty() || college.isEmpty() || major.isEmpty() || grade.isEmpty()) {
        statusLabel_->setText(QStringLiteral("请填写所有字段"));
        return;
    }
    if (attachmentId_.isEmpty()) {
        statusLabel_->setText(QStringLiteral("请先上传认证材料"));
        return;
    }

    submitButton_->setEnabled(false);
    statusLabel_->setText(QStringLiteral("提交中..."));

    authService_.submitIdentityVerification(realName, studentNumber, college, major, grade, attachmentId_, [this](const AuthResult &result) {
        submitButton_->setEnabled(true);
        if (result.success) {
            statusLabel_->setText(QStringLiteral("提交成功，等待审核"));
            attachmentId_.clear();
            emit submissionSuccess();
        } else {
            statusLabel_->setText(result.errorMessage.isEmpty() ? QStringLiteral("提交失败") : result.errorMessage);
        }
    });
}
