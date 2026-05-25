#pragma once

#include <QComboBox>
#include <QHash>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QTextEdit>
#include <QWidget>

#include "api/MyPartnerPostApiService.h"

class PostEditorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PostEditorWidget(MyPartnerPostApiService &myPostService, QWidget *parent = nullptr);

    void loadPost(const QString &postId, const MyPostItem &item);
    void clearForm();

signals:
    void postSaved();
    void postSubmitted();

private slots:
    void onSaveDraft();
    void onUpdateDraft();
    void onSubmitReview();
    void onSceneTypeChanged();

private:
    MyPostDraftRequest buildDraftRequest() const;
    void setStatusMessage(const QString &msg);
    void clearFieldErrors();
    void applyFieldErrors(const QJsonObject &details);
    QLabel *createFieldErrorLabel(QWidget *parent);
    QLabel *fieldErrorLabelForKey(const QString &key) const;
    static QString fieldDisplayName(const QString &key);
    static QString formatErrorDetails(const QJsonObject &details);
    static QString sceneFieldKey(const QString &sceneType);
    static QString sceneFieldLabel(const QString &sceneType);

    MyPartnerPostApiService &myPostService_;

    QComboBox *sceneTypeCombo_;
    QLineEdit *titleEdit_;
    QTextEdit *descriptionEdit_;
    QComboBox *timeModeCombo_;
    QLineEdit *timeTextEdit_;
    QLineEdit *locationTextEdit_;
    QSpinBox *participantCountSpin_;
    QLineEdit *targetRequirementEdit_;
    QLineEdit *contactPreferenceEdit_;
    QLineEdit *tagsEdit_;
    QLineEdit *sceneFieldEdit_;
    QLabel *sceneFieldLabel_;
    QLabel *titleErrorLabel_;
    QLabel *descriptionErrorLabel_;
    QLabel *sceneFieldErrorLabel_;
    QLabel *timeTextErrorLabel_;
    QLabel *locationTextErrorLabel_;
    QLabel *targetRequirementErrorLabel_;
    QLabel *contactPreferenceErrorLabel_;
    QHash<QString, QLabel *> fieldErrorLabels_;

    QPushButton *saveDraftButton_;
    QPushButton *updateDraftButton_;
    QPushButton *submitReviewButton_;
    QLabel *statusLabel_;
    QLabel *postIdLabel_;

    QString currentPostId_;
    bool submitting_ = false;
};
