#pragma once

#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QString>
#include <QStringList>
#include <QWidget>

namespace UiHelpers
{
QFrame *createPageHeader(const QString &title, const QString &subtitle, QWidget *parent);
QFrame *createCard(QWidget *parent);
QLabel *createSectionTitle(const QString &text, QWidget *parent);
QLabel *createStatusLabel(QWidget *parent);
QPushButton *markPrimary(QPushButton *button);
QPushButton *markSecondary(QPushButton *button);
QPushButton *markDanger(QPushButton *button);
QPushButton *markGhost(QPushButton *button);
QString sceneDisplayName(const QString &sceneType);
QString statusDisplayName(const QString &status);
QString contactUnlockStatusText(const QString &status);
QString compactTags(const QStringList &tags);
}
