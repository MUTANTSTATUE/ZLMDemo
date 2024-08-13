/********************************************************************************
** Form generated from reading UI file 'widget.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WIDGET_H
#define UI_WIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Widget
{
public:
    QVBoxLayout *verticalLayout;
    QWidget *widget;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label;
    QLineEdit *lineEditPushUrl;
    QPushButton *pushButtonPush;
    QHBoxLayout *horizontalLayout;
    QLabel *label_2;
    QLineEdit *lineEditPlayUrl;
    QPushButton *pushButtonPlay;

    void setupUi(QWidget *Widget)
    {
        if (Widget->objectName().isEmpty())
            Widget->setObjectName(QString::fromUtf8("Widget"));
        Widget->resize(800, 600);
        verticalLayout = new QVBoxLayout(Widget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        widget = new QWidget(Widget);
        widget->setObjectName(QString::fromUtf8("widget"));

        verticalLayout->addWidget(widget);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label = new QLabel(Widget);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout_2->addWidget(label);

        lineEditPushUrl = new QLineEdit(Widget);
        lineEditPushUrl->setObjectName(QString::fromUtf8("lineEditPushUrl"));
        lineEditPushUrl->setReadOnly(true);

        horizontalLayout_2->addWidget(lineEditPushUrl);

        pushButtonPush = new QPushButton(Widget);
        pushButtonPush->setObjectName(QString::fromUtf8("pushButtonPush"));
        pushButtonPush->setCheckable(true);

        horizontalLayout_2->addWidget(pushButtonPush);


        verticalLayout->addLayout(horizontalLayout_2);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label_2 = new QLabel(Widget);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout->addWidget(label_2);

        lineEditPlayUrl = new QLineEdit(Widget);
        lineEditPlayUrl->setObjectName(QString::fromUtf8("lineEditPlayUrl"));

        horizontalLayout->addWidget(lineEditPlayUrl);

        pushButtonPlay = new QPushButton(Widget);
        pushButtonPlay->setObjectName(QString::fromUtf8("pushButtonPlay"));

        horizontalLayout->addWidget(pushButtonPlay);


        verticalLayout->addLayout(horizontalLayout);

        verticalLayout->setStretch(0, 1);

        retranslateUi(Widget);

        QMetaObject::connectSlotsByName(Widget);
    } // setupUi

    void retranslateUi(QWidget *Widget)
    {
        Widget->setWindowTitle(QCoreApplication::translate("Widget", "Widget", nullptr));
        label->setText(QCoreApplication::translate("Widget", "\346\216\250\346\265\201\345\234\260\345\235\200\357\274\232", nullptr));
        pushButtonPush->setText(QCoreApplication::translate("Widget", "\344\270\212\344\274\240", nullptr));
        label_2->setText(QCoreApplication::translate("Widget", "\346\222\255\346\224\276\345\234\260\345\235\200\357\274\232", nullptr));
        lineEditPlayUrl->setText(QCoreApplication::translate("Widget", "rtsp://127.0.0.1/live/test", nullptr));
        pushButtonPlay->setText(QCoreApplication::translate("Widget", "\346\222\255\346\224\276", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Widget: public Ui_Widget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WIDGET_H
