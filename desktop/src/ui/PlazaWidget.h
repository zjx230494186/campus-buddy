#pragma once

#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QWidget>

#include "api/PartnerPostApiService.h"
#include "api/ContactConversationApiService.h"

class PlazaWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PlazaWidget(PartnerPostApiService &plazaService,
                         ContactConversationApiService &contactService,
                         QWidget *parent = nullptr);

private slots:
    void onRefresh();
    void onItemSelected();
    void onContactRequest();

private:
    PartnerPostApiService &plazaService_;
    ContactConversationApiService &contactService_;

    QComboBox *sceneTypeFilter_;
    QLineEdit *keywordFilter_;
    QPushButton *refreshButton_;
    QListWidget *listWidget_;
    QLabel *detailLabel_;
    QLineEdit *contactMessageEdit_;
    QPushButton *contactButton_;
    QLabel *statusLabel_;

    QList<PlazaListItem> items_;
    int selectedIndex_ = -1;
};
