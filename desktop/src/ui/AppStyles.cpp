#include "ui/AppStyles.h"

namespace AppStyles
{

void apply(QApplication &app)
{
    app.setStyleSheet(styleSheet());
}

QString styleSheet()
{
    return QStringLiteral(R"(
        QWidget {
            background: #f6f8f7;
            color: #1f2933;
            font-family: "Microsoft YaHei", "Segoe UI", sans-serif;
            font-size: 13px;
        }

        QFrame#pageHeader, QFrame[card="true"] {
            background: #ffffff;
            border: 1px solid #d9e2df;
            border-radius: 8px;
        }

        QLabel#pageTitle {
            color: #102a43;
            font-size: 22px;
            font-weight: 700;
        }

        QLabel#pageSubtitle, QLabel[muted="true"] {
            color: #627d78;
        }

        QLabel[sectionTitle="true"] {
            color: #173f35;
            font-size: 15px;
            font-weight: 700;
            background: transparent;
        }

        QLabel[status="true"] {
            background: #eef7f5;
            color: #285c53;
            border: 1px solid #c6e4dd;
            border-radius: 6px;
            padding: 7px 10px;
        }

        QLineEdit, QTextEdit, QComboBox, QSpinBox {
            background: #ffffff;
            border: 1px solid #cfd9d6;
            border-radius: 6px;
            padding: 7px 9px;
            selection-background-color: #1f9d8a;
        }

        QTextEdit {
            min-height: 72px;
        }

        QLineEdit:focus, QTextEdit:focus, QComboBox:focus, QSpinBox:focus {
            border: 1px solid #1f9d8a;
        }

        QPushButton {
            background: #ffffff;
            border: 1px solid #b8c7c3;
            border-radius: 6px;
            padding: 8px 14px;
            min-height: 18px;
        }

        QPushButton:hover {
            background: #f0f6f4;
        }

        QPushButton[role="primary"] {
            background: #168a78;
            border-color: #168a78;
            color: #ffffff;
            font-weight: 700;
        }

        QPushButton[role="primary"]:hover {
            background: #0f7668;
        }

        QPushButton[role="secondary"] {
            background: #eaf4f1;
            border-color: #9ac9be;
            color: #175c50;
        }

        QPushButton[role="danger"] {
            background: #fff5f5;
            border-color: #f0b4b4;
            color: #b42318;
        }

        QPushButton[role="ghost"] {
            background: transparent;
            border-color: transparent;
            color: #176b87;
        }

        QPushButton:disabled {
            background: #edf1f0;
            border-color: #d7dfdd;
            color: #8b9a97;
        }

        QTabWidget::pane {
            border: 1px solid #d9e2df;
            border-radius: 8px;
            background: #ffffff;
        }

        QTabBar::tab {
            background: #e9efed;
            color: #38534f;
            padding: 9px 16px;
            border-top-left-radius: 6px;
            border-top-right-radius: 6px;
            margin-right: 3px;
        }

        QTabBar::tab:selected {
            background: #ffffff;
            color: #0f5f55;
            font-weight: 700;
        }

        QListWidget {
            background: #ffffff;
            border: 1px solid #d9e2df;
            border-radius: 8px;
            padding: 4px;
        }

        QListWidget::item {
            border-radius: 6px;
            padding: 8px;
            margin: 2px;
        }

        QListWidget::item:selected {
            background: #dff3ef;
            color: #113f38;
        }

        QGroupBox {
            background: #ffffff;
            border: 1px solid #d9e2df;
            border-radius: 8px;
            margin-top: 12px;
            padding: 12px 10px 10px 10px;
            font-weight: 700;
            color: #173f35;
        }

        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 4px;
        }
    )");
}

}
