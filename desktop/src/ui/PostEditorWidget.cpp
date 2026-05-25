#include "ui/PostEditorWidget.h"
#include "ui/UiHelpers.h"

#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QScrollArea>
#include <QVBoxLayout>

PostEditorWidget::PostEditorWidget(MyPartnerPostApiService &myPostService, QWidget *parent)
    : QWidget(parent),
      myPostService_(myPostService)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(12);

    layout->addWidget(UiHelpers::createPageHeader(
        QStringLiteral("发布搭子需求"),
        QStringLiteral("先保存草稿，确认内容清楚后再提交审核。联系方式请留在会话交换卡片中。"),
        this));

    postIdLabel_ = new QLabel(this);
    postIdLabel_->setObjectName(QStringLiteral("postIdLabel"));
    postIdLabel_->setProperty("muted", true);
    layout->addWidget(postIdLabel_);

    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    auto *formHost = new QWidget(scrollArea);
    auto *formHostLayout = new QVBoxLayout(formHost);
    formHostLayout->setContentsMargins(0, 0, 0, 0);
    formHostLayout->setSpacing(12);

    auto *basicGroup = new QGroupBox(QStringLiteral("基础信息"), formHost);
    auto *basicForm = new QFormLayout(basicGroup);
    basicForm->setLabelAlignment(Qt::AlignRight);

    sceneTypeCombo_ = new QComboBox(this);
    sceneTypeCombo_->setObjectName(QStringLiteral("sceneTypeCombo"));
    sceneTypeCombo_->addItems({QStringLiteral("STUDY"), QStringLiteral("MEAL"), QStringLiteral("SPORT"),
                               QStringLiteral("COURSE_TEAM"), QStringLiteral("INNOVATION_PROJECT")});
    basicForm->addRow(QStringLiteral("场景类型"), sceneTypeCombo_);

    titleEdit_ = new QLineEdit(this);
    titleEdit_->setObjectName(QStringLiteral("titleEdit"));
    titleEdit_->setPlaceholderText(QStringLiteral("一句话说明你想找什么搭子"));
    basicForm->addRow(QStringLiteral("标题"), titleEdit_);

    descriptionEdit_ = new QTextEdit(this);
    descriptionEdit_->setObjectName(QStringLiteral("descriptionEdit"));
    descriptionEdit_->setPlaceholderText(QStringLiteral("补充目标、节奏、人数预期和注意事项"));
    descriptionEdit_->setMinimumHeight(96);
    basicForm->addRow(QStringLiteral("正文"), descriptionEdit_);

    sceneFieldLabel_ = new QLabel(QStringLiteral("学习目标"), this);
    sceneFieldEdit_ = new QLineEdit(this);
    sceneFieldEdit_->setObjectName(QStringLiteral("sceneFieldEdit"));
    sceneFieldEdit_->setPlaceholderText(QStringLiteral("如: 通过考试"));
    basicForm->addRow(sceneFieldLabel_, sceneFieldEdit_);
    formHostLayout->addWidget(basicGroup);

    auto *timeGroup = new QGroupBox(QStringLiteral("时间与地点"), formHost);
    auto *timeForm = new QFormLayout(timeGroup);
    timeForm->setLabelAlignment(Qt::AlignRight);

    timeModeCombo_ = new QComboBox(this);
    timeModeCombo_->setObjectName(QStringLiteral("timeModeCombo"));
    timeModeCombo_->addItems({QStringLiteral("EXACT_TIME"), QStringLiteral("TIME_RANGE"), QStringLiteral("TEXT_PREFERENCE")});
    timeModeCombo_->setCurrentIndex(2);
    timeForm->addRow(QStringLiteral("时间模式"), timeModeCombo_);

    timeTextEdit_ = new QLineEdit(this);
    timeTextEdit_->setObjectName(QStringLiteral("timeTextEdit"));
    timeTextEdit_->setPlaceholderText(QStringLiteral("如: 周三晚 7 点，或工作日晚上"));
    timeForm->addRow(QStringLiteral("时间"), timeTextEdit_);

    locationTextEdit_ = new QLineEdit(this);
    locationTextEdit_->setObjectName(QStringLiteral("locationTextEdit"));
    locationTextEdit_->setPlaceholderText(QStringLiteral("如: 图书馆三楼 / 一食堂"));
    timeForm->addRow(QStringLiteral("地点"), locationTextEdit_);

    participantCountSpin_ = new QSpinBox(this);
    participantCountSpin_->setObjectName(QStringLiteral("participantCountSpin"));
    participantCountSpin_->setRange(1, 100);
    participantCountSpin_->setValue(2);
    timeForm->addRow(QStringLiteral("人数"), participantCountSpin_);
    formHostLayout->addWidget(timeGroup);

    auto *preferenceGroup = new QGroupBox(QStringLiteral("偏好与标签"), formHost);
    auto *preferenceForm = new QFormLayout(preferenceGroup);
    preferenceForm->setLabelAlignment(Qt::AlignRight);

    targetRequirementEdit_ = new QLineEdit(this);
    targetRequirementEdit_->setObjectName(QStringLiteral("targetRequirementEdit"));
    targetRequirementEdit_->setPlaceholderText(QStringLiteral("如: 希望对方准时、基础相近"));
    preferenceForm->addRow(QStringLiteral("要求"), targetRequirementEdit_);

    contactPreferenceEdit_ = new QLineEdit(this);
    contactPreferenceEdit_->setObjectName(QStringLiteral("contactPreferenceEdit"));
    contactPreferenceEdit_->setPlaceholderText(QStringLiteral("如: 先站内聊，确认后交换联系方式"));
    preferenceForm->addRow(QStringLiteral("联系偏好"), contactPreferenceEdit_);

    tagsEdit_ = new QLineEdit(this);
    tagsEdit_->setObjectName(QStringLiteral("tagsEdit"));
    tagsEdit_->setPlaceholderText(QStringLiteral("逗号分隔，如 高数,考研,羽毛球"));
    preferenceForm->addRow(QStringLiteral("标签"), tagsEdit_);
    formHostLayout->addWidget(preferenceGroup);
    formHostLayout->addStretch();
    scrollArea->setWidget(formHost);
    layout->addWidget(scrollArea, 1);

    auto *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    saveDraftButton_ = UiHelpers::markSecondary(new QPushButton(QStringLiteral("保存草稿"), this));
    saveDraftButton_->setObjectName(QStringLiteral("saveDraftButton"));
    buttonLayout->addWidget(saveDraftButton_);

    updateDraftButton_ = UiHelpers::markSecondary(new QPushButton(QStringLiteral("更新草稿"), this));
    updateDraftButton_->setObjectName(QStringLiteral("updateDraftButton"));
    updateDraftButton_->setEnabled(false);
    buttonLayout->addWidget(updateDraftButton_);

    submitReviewButton_ = UiHelpers::markPrimary(new QPushButton(QStringLiteral("提交审核"), this));
    submitReviewButton_->setObjectName(QStringLiteral("submitReviewButton"));
    submitReviewButton_->setEnabled(false);
    buttonLayout->addWidget(submitReviewButton_);
    layout->addLayout(buttonLayout);

    statusLabel_ = UiHelpers::createStatusLabel(this);
    statusLabel_->setObjectName(QStringLiteral("statusLabel"));
    layout->addWidget(statusLabel_);

    connect(saveDraftButton_, &QPushButton::clicked, this, &PostEditorWidget::onSaveDraft);
    connect(updateDraftButton_, &QPushButton::clicked, this, &PostEditorWidget::onUpdateDraft);
    connect(submitReviewButton_, &QPushButton::clicked, this, &PostEditorWidget::onSubmitReview);
    connect(sceneTypeCombo_, &QComboBox::currentTextChanged, this, &PostEditorWidget::onSceneTypeChanged);

    onSceneTypeChanged();
}

QString PostEditorWidget::sceneFieldKey(const QString &sceneType)
{
    if (sceneType == QStringLiteral("MEAL")) return QStringLiteral("canteen");
    if (sceneType == QStringLiteral("STUDY")) return QStringLiteral("studyGoal");
    if (sceneType == QStringLiteral("SPORT")) return QStringLiteral("sportType");
    if (sceneType == QStringLiteral("COURSE_TEAM")) return QStringLiteral("courseName");
    if (sceneType == QStringLiteral("INNOVATION_PROJECT")) return QStringLiteral("projectDirection");
    return QString();
}

QString PostEditorWidget::sceneFieldLabel(const QString &sceneType)
{
    if (sceneType == QStringLiteral("MEAL")) return QStringLiteral("食堂");
    if (sceneType == QStringLiteral("STUDY")) return QStringLiteral("学习目标");
    if (sceneType == QStringLiteral("SPORT")) return QStringLiteral("运动类型");
    if (sceneType == QStringLiteral("COURSE_TEAM")) return QStringLiteral("课程名称");
    if (sceneType == QStringLiteral("INNOVATION_PROJECT")) return QStringLiteral("项目方向");
    return QStringLiteral("场景字段");
}

void PostEditorWidget::onSceneTypeChanged()
{
    const QString sceneType = sceneTypeCombo_->currentText();
    sceneFieldLabel_->setText(sceneFieldLabel(sceneType));
    const QString key = sceneFieldKey(sceneType);
    if (key == QStringLiteral("canteen")) {
        sceneFieldEdit_->setPlaceholderText(QStringLiteral("如: 一食堂"));
    } else if (key == QStringLiteral("studyGoal")) {
        sceneFieldEdit_->setPlaceholderText(QStringLiteral("如: 通过考试"));
    } else if (key == QStringLiteral("sportType")) {
        sceneFieldEdit_->setPlaceholderText(QStringLiteral("如: 篮球"));
    } else if (key == QStringLiteral("courseName")) {
        sceneFieldEdit_->setPlaceholderText(QStringLiteral("如: 高等数学"));
    } else if (key == QStringLiteral("projectDirection")) {
        sceneFieldEdit_->setPlaceholderText(QStringLiteral("如: AI应用"));
    }
}

QString PostEditorWidget::formatErrorDetails(const QJsonObject &details)
{
    if (details.isEmpty()) return QString();
    QStringList parts;
    for (auto it = details.constBegin(); it != details.constEnd(); ++it) {
        QString key = it.key();
        if (key.startsWith(QStringLiteral("scenePayload."))) {
            key = key.mid(QStringLiteral("scenePayload.").length());
        }
        QString val;
        if (it.value().isString()) {
            val = it.value().toString();
        } else {
            val = QString::fromUtf8(QJsonDocument(it.value().toObject()).toJson(QJsonDocument::Compact));
        }
        if (val.isEmpty()) val = QStringLiteral("不合法");
        parts.append(QStringLiteral("%1: %2").arg(key, val));
    }
    return parts.join(QStringLiteral("; "));
}

void PostEditorWidget::loadPost(const QString &postId, const MyPostItem &item)
{
    currentPostId_ = postId;
    postIdLabel_->setText(QStringLiteral("当前草稿: %1 / %2")
        .arg(postId.left(8) + QStringLiteral("..."), UiHelpers::statusDisplayName(item.status)));

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

    const QString key = sceneFieldKey(item.sceneType);
    if (!key.isEmpty() && item.scenePayload.contains(key)) {
        sceneFieldEdit_->setText(item.scenePayload.value(key).toString());
    } else {
        sceneFieldEdit_->clear();
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
    sceneFieldEdit_->clear();
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

    const QString key = sceneFieldKey(req.sceneType);
    if (!key.isEmpty() && !sceneFieldEdit_->text().isEmpty()) {
        req.scenePayload.insert(key, sceneFieldEdit_->text());
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
            postIdLabel_->setText(QStringLiteral("当前草稿: %1 / %2").arg(currentPostId_.left(8) + QStringLiteral("..."), UiHelpers::statusDisplayName(result.post.status)));
            updateDraftButton_->setEnabled(result.post.allowedActions.contains(QStringLiteral("UPDATE_DRAFT")));
            submitReviewButton_->setEnabled(result.post.allowedActions.contains(QStringLiteral("SUBMIT_REVIEW")));
            saveDraftButton_->setEnabled(false);
            setStatusMessage(QStringLiteral("草稿已保存"));
            emit postSaved();
        } else {
            QString msg = QStringLiteral("保存失败: %1 - %2").arg(result.errorCode, result.errorMessage);
            QString details = formatErrorDetails(result.errorDetails);
            if (!details.isEmpty()) msg += QStringLiteral(" (%1)").arg(details);
            setStatusMessage(msg);
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
            postIdLabel_->setText(QStringLiteral("当前草稿: %1 / %2").arg(currentPostId_.left(8) + QStringLiteral("..."), UiHelpers::statusDisplayName(result.post.status)));
            updateDraftButton_->setEnabled(result.post.allowedActions.contains(QStringLiteral("UPDATE_DRAFT")));
            submitReviewButton_->setEnabled(result.post.allowedActions.contains(QStringLiteral("SUBMIT_REVIEW")));
            setStatusMessage(QStringLiteral("草稿已更新"));
            emit postSaved();
        } else {
            QString msg = QStringLiteral("更新失败: %1 - %2").arg(result.errorCode, result.errorMessage);
            QString details = formatErrorDetails(result.errorDetails);
            if (!details.isEmpty()) msg += QStringLiteral(" (%1)").arg(details);
            setStatusMessage(msg);
        }
    });
}

void PostEditorWidget::onSubmitReview()
{
    if (currentPostId_.isEmpty()) return;
    if (submitting_) return;
    submitting_ = true;
    submitReviewButton_->setEnabled(false);
    setStatusMessage(QStringLiteral("提交审核中..."));
    myPostService_.submitReview(currentPostId_, [this](const PostActionResult &result) {
        submitting_ = false;
        if (result.success) {
            postIdLabel_->setText(QStringLiteral("当前草稿: %1 / %2").arg(currentPostId_.left(8) + QStringLiteral("..."), UiHelpers::statusDisplayName(result.post.status)));
            updateDraftButton_->setEnabled(result.post.allowedActions.contains(QStringLiteral("UPDATE_DRAFT")));
            submitReviewButton_->setEnabled(result.post.allowedActions.contains(QStringLiteral("SUBMIT_REVIEW")));
            setStatusMessage(QStringLiteral("已提交审核"));
            emit postSubmitted();
        } else {
            QString msg;
            if (result.errorCode == QStringLiteral("VALIDATION_FAILED")) {
                msg = QStringLiteral("校验失败");
                QString details = formatErrorDetails(result.errorDetails);
                if (!details.isEmpty()) {
                    msg += QStringLiteral(": %1").arg(details);
                } else if (!result.errorMessage.isEmpty()) {
                    msg += QStringLiteral(": %1").arg(result.errorMessage);
                }
            } else if (result.errorCode == QStringLiteral("POST_STATUS_CONFLICT")) {
                msg = QStringLiteral("状态冲突: %1").arg(result.errorMessage);
            } else if (result.httpStatus == 401) {
                msg = QStringLiteral("登录已过期，请重新登录");
            } else if (result.httpStatus == 403) {
                msg = QStringLiteral("权限不足: %1").arg(result.errorMessage);
            } else if (result.httpStatus == 0) {
                msg = QStringLiteral("网络连接失败，请检查服务器地址");
            } else {
                msg = QStringLiteral("提交失败: %1 - %2").arg(result.errorCode, result.errorMessage);
                QString details = formatErrorDetails(result.errorDetails);
                if (!details.isEmpty()) msg += QStringLiteral(" (%1)").arg(details);
            }
            setStatusMessage(msg);
            submitReviewButton_->setEnabled(true);
        }
    });
}

void PostEditorWidget::setStatusMessage(const QString &msg)
{
    statusLabel_->setText(msg);
}
