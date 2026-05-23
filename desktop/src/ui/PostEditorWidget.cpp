#include "ui/PostEditorWidget.h"

#include <QFormLayout>
#include <QJsonArray>
#include <QVBoxLayout>

PostEditorWidget::PostEditorWidget(MyPartnerPostApiService &myPostService, QWidget *parent)
    : QWidget(parent),
      myPostService_(myPostService)
{
    auto *layout = new QVBoxLayout(this);

    postIdLabel_ = new QLabel(this);
    postIdLabel_->setObjectName(QStringLiteral("postIdLabel"));
    layout->addWidget(postIdLabel_);

    auto *form = new QFormLayout();

    sceneTypeCombo_ = new QComboBox(this);
    sceneTypeCombo_->setObjectName(QStringLiteral("sceneTypeCombo"));
    sceneTypeCombo_->addItems({QStringLiteral("STUDY"), QStringLiteral("SPORT"), QStringLiteral("TRAVEL"),
                               QStringLiteral("FOOD"), QStringLiteral("OTHER")});
    form->addRow(QStringLiteral("场景类型"), sceneTypeCombo_);

    titleEdit_ = new QLineEdit(this);
    titleEdit_->setObjectName(QStringLiteral("titleEdit"));
    form->addRow(QStringLiteral("标题"), titleEdit_);

    descriptionEdit_ = new QTextEdit(this);
    descriptionEdit_->setObjectName(QStringLiteral("descriptionEdit"));
    descriptionEdit_->setMaximumHeight(80);
    form->addRow(QStringLiteral("描述"), descriptionEdit_);

    timeModeCombo_ = new QComboBox(this);
    timeModeCombo_->setObjectName(QStringLiteral("timeModeCombo"));
    timeModeCombo_->addItems({QStringLiteral("EXACT_TIME"), QStringLiteral("TIME_RANGE"), QStringLiteral("TEXT_PREFERENCE")});
    timeModeCombo_->setCurrentIndex(2);
    form->addRow(QStringLiteral("时间模式"), timeModeCombo_);

    timeTextEdit_ = new QLineEdit(this);
    timeTextEdit_->setObjectName(QStringLiteral("timeTextEdit"));
    form->addRow(QStringLiteral("时间"), timeTextEdit_);

    locationTextEdit_ = new QLineEdit(this);
    locationTextEdit_->setObjectName(QStringLiteral("locationTextEdit"));
    form->addRow(QStringLiteral("地点"), locationTextEdit_);

    participantCountSpin_ = new QSpinBox(this);
    participantCountSpin_->setObjectName(QStringLiteral("participantCountSpin"));
    participantCountSpin_->setRange(1, 100);
    participantCountSpin_->setValue(2);
    form->addRow(QStringLiteral("人数"), participantCountSpin_);

    targetRequirementEdit_ = new QLineEdit(this);
    targetRequirementEdit_->setObjectName(QStringLiteral("targetRequirementEdit"));
    form->addRow(QStringLiteral("要求"), targetRequirementEdit_);

    contactPreferenceEdit_ = new QLineEdit(this);
    contactPreferenceEdit_->setObjectName(QStringLiteral("contactPreferenceEdit"));
    contactPreferenceEdit_->setPlaceholderText(QStringLiteral("如: in-app chat"));
    form->addRow(QStringLiteral("联系偏好"), contactPreferenceEdit_);

    tagsEdit_ = new QLineEdit(this);
    tagsEdit_->setObjectName(QStringLiteral("tagsEdit"));
    tagsEdit_->setPlaceholderText(QStringLiteral("逗号分隔，如 math,physics"));
    form->addRow(QStringLiteral("标签"), tagsEdit_);

    studyGoalEdit_ = new QLineEdit(this);
    studyGoalEdit_->setObjectName(QStringLiteral("studyGoalEdit"));
    form->addRow(QStringLiteral("学习目标"), studyGoalEdit_);

    layout->addLayout(form);

    saveDraftButton_ = new QPushButton(QStringLiteral("保存草稿"), this);
    saveDraftButton_->setObjectName(QStringLiteral("saveDraftButton"));
    layout->addWidget(saveDraftButton_);

    updateDraftButton_ = new QPushButton(QStringLiteral("更新草稿"), this);
    updateDraftButton_->setObjectName(QStringLiteral("updateDraftButton"));
    updateDraftButton_->setEnabled(false);
    layout->addWidget(updateDraftButton_);

    submitReviewButton_ = new QPushButton(QStringLiteral("提交审核"), this);
    submitReviewButton_->setObjectName(QStringLiteral("submitReviewButton"));
    submitReviewButton_->setEnabled(false);
    layout->addWidget(submitReviewButton_);

    statusLabel_ = new QLabel(this);
    statusLabel_->setObjectName(QStringLiteral("statusLabel"));
    layout->addWidget(statusLabel_);

    connect(saveDraftButton_, &QPushButton::clicked, this, &PostEditorWidget::onSaveDraft);
    connect(updateDraftButton_, &QPushButton::clicked, this, &PostEditorWidget::onUpdateDraft);
    connect(submitReviewButton_, &QPushButton::clicked, this, &PostEditorWidget::onSubmitReview);
}

void PostEditorWidget::loadPost(const QString &postId, const MyPostItem &item)
{
    currentPostId_ = postId;
    postIdLabel_->setText(QStringLiteral("Post ID: %1  Status: %2").arg(postId.left(8) + QStringLiteral("...")).arg(item.status));

    sceneTypeCombo_->setCurrentText(item.sceneType);
    titleEdit_->setText(item.title);
    descriptionEdit_->setPlainText(item.description);
    timeModeCombo_->setCurrentText(item.timeMode);
    timeTextEdit_->setText(item.timeText);
    locationTextEdit_->setText(item.locationText);
    participantCountSpin_->setValue(item.participantCount > 0 ? item.participantCount : 2);
    targetRequirementEdit_->setText(item.targetRequirement);
    contactPreferenceEdit_->setText(item.contactPreference);
    tagsEdit_->setText(item.tags.join(QStringLiteral(",")));
    if (item.scenePayload.contains(QStringLiteral("studyGoal"))) {
        studyGoalEdit_->setText(item.scenePayload.value(QStringLiteral("studyGoal")).toString());
    }

    updateDraftButton_->setEnabled(item.allowedActions.contains(QStringLiteral("UPDATE_DRAFT")));
    submitReviewButton_->setEnabled(item.allowedActions.contains(QStringLiteral("SUBMIT_REVIEW")));
    saveDraftButton_->setEnabled(false);
}

void PostEditorWidget::clearForm()
{
    currentPostId_.clear();
    postIdLabel_->clear();
    titleEdit_->clear();
    descriptionEdit_->clear();
    timeTextEdit_->clear();
    locationTextEdit_->clear();
    targetRequirementEdit_->clear();
    contactPreferenceEdit_->clear();
    tagsEdit_->clear();
    studyGoalEdit_->clear();
    statusLabel_->clear();
    saveDraftButton_->setEnabled(true);
    updateDraftButton_->setEnabled(false);
    submitReviewButton_->setEnabled(false);
}

MyPostDraftRequest PostEditorWidget::buildDraftRequest() const
{
    MyPostDraftRequest req;
    req.sceneType = sceneTypeCombo_->currentText();
    req.title = titleEdit_->text();
    req.description = descriptionEdit_->toPlainText();
    req.timeMode = timeModeCombo_->currentText();
    req.timeText = timeTextEdit_->text();
    req.locationText = locationTextEdit_->text();
    req.participantCount = participantCountSpin_->value();
    req.targetRequirement = targetRequirementEdit_->text();
    req.contactPreference = contactPreferenceEdit_->text();
    req.tags = tagsEdit_->text().split(QStringLiteral(","), Qt::SkipEmptyParts);
    if (!studyGoalEdit_->text().isEmpty()) {
        req.scenePayload.insert(QStringLiteral("studyGoal"), studyGoalEdit_->text());
    }
    return req;
}

void PostEditorWidget::onSaveDraft()
{
    setStatusMessage(QStringLiteral("保存中..."));
    MyPostDraftRequest req = buildDraftRequest();
    myPostService_.createDraft(req, [this](const MyPostResult &result) {
        if (result.success) {
            currentPostId_ = result.post.postId;
            postIdLabel_->setText(QStringLiteral("Post ID: %1  Status: %2").arg(currentPostId_.left(8) + QStringLiteral("...")).arg(result.post.status));
            updateDraftButton_->setEnabled(result.post.allowedActions.contains(QStringLiteral("UPDATE_DRAFT")));
            submitReviewButton_->setEnabled(result.post.allowedActions.contains(QStringLiteral("SUBMIT_REVIEW")));
            saveDraftButton_->setEnabled(false);
            setStatusMessage(QStringLiteral("草稿已保存"));
            emit postSaved();
        } else {
            setStatusMessage(QStringLiteral("保存失败: %1").arg(result.errorMessage));
        }
    });
}

void PostEditorWidget::onUpdateDraft()
{
    if (currentPostId_.isEmpty()) return;
    setStatusMessage(QStringLiteral("更新中..."));
    MyPostDraftRequest req = buildDraftRequest();
    myPostService_.updateDraft(currentPostId_, req, [this](const MyPostResult &result) {
        if (result.success) {
            postIdLabel_->setText(QStringLiteral("Post ID: %1  Status: %2").arg(currentPostId_.left(8) + QStringLiteral("...")).arg(result.post.status));
            updateDraftButton_->setEnabled(result.post.allowedActions.contains(QStringLiteral("UPDATE_DRAFT")));
            submitReviewButton_->setEnabled(result.post.allowedActions.contains(QStringLiteral("SUBMIT_REVIEW")));
            setStatusMessage(QStringLiteral("草稿已更新"));
            emit postSaved();
        } else {
            setStatusMessage(QStringLiteral("更新失败: %1 - %2").arg(result.errorCode).arg(result.errorMessage));
        }
    });
}

void PostEditorWidget::onSubmitReview()
{
    if (currentPostId_.isEmpty()) return;
    setStatusMessage(QStringLiteral("提交审核中..."));
    myPostService_.submitReview(currentPostId_, [this](const PostActionResult &result) {
        if (result.success) {
            postIdLabel_->setText(QStringLiteral("Post ID: %1  Status: %2").arg(currentPostId_.left(8) + QStringLiteral("...")).arg(result.post.status));
            updateDraftButton_->setEnabled(result.post.allowedActions.contains(QStringLiteral("UPDATE_DRAFT")));
            submitReviewButton_->setEnabled(result.post.allowedActions.contains(QStringLiteral("SUBMIT_REVIEW")));
            setStatusMessage(QStringLiteral("已提交审核"));
            emit postSubmitted();
        } else {
            setStatusMessage(QStringLiteral("提交失败: %1 - %2").arg(result.errorCode).arg(result.errorMessage));
        }
    });
}

void PostEditorWidget::setStatusMessage(const QString &msg)
{
    statusLabel_->setText(msg);
}
