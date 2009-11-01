/********************************************************************************
** Form generated from reading ui file 'OverlapViewer.ui'
**
** Created: Sun Nov 1 18:07:24 2009
**      by: Qt User Interface Compiler version 4.5.3
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_OVERLAPVIEWER_H
#define UI_OVERLAPVIEWER_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>
#include <QtGui/QListWidget>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QSplitter>
#include <QtGui/QStatusBar>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_OverlapMainWindow
{
public:
    QAction *actionQuit;
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout_3;
    QSplitter *splitter;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout;
    QListWidget *featuresListWidget;
    QGroupBox *groupBox_2;
    QVBoxLayout *verticalLayout_2;
    QListWidget *overlappingListWidget;
    QMenuBar *menubar;
    QMenu *menuFile;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *OverlapMainWindow)
    {
        if (OverlapMainWindow->objectName().isEmpty())
            OverlapMainWindow->setObjectName(QString::fromUtf8("OverlapMainWindow"));
        OverlapMainWindow->resize(800, 600);
        actionQuit = new QAction(OverlapMainWindow);
        actionQuit->setObjectName(QString::fromUtf8("actionQuit"));
        centralwidget = new QWidget(OverlapMainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        verticalLayout_3 = new QVBoxLayout(centralwidget);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        splitter = new QSplitter(centralwidget);
        splitter->setObjectName(QString::fromUtf8("splitter"));
        splitter->setOrientation(Qt::Horizontal);
        groupBox = new QGroupBox(splitter);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        verticalLayout = new QVBoxLayout(groupBox);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        featuresListWidget = new QListWidget(groupBox);
        featuresListWidget->setObjectName(QString::fromUtf8("featuresListWidget"));

        verticalLayout->addWidget(featuresListWidget);

        splitter->addWidget(groupBox);
        groupBox_2 = new QGroupBox(splitter);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        verticalLayout_2 = new QVBoxLayout(groupBox_2);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        overlappingListWidget = new QListWidget(groupBox_2);
        overlappingListWidget->setObjectName(QString::fromUtf8("overlappingListWidget"));

        verticalLayout_2->addWidget(overlappingListWidget);

        splitter->addWidget(groupBox_2);

        verticalLayout_3->addWidget(splitter);

        OverlapMainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(OverlapMainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 800, 22));
        menuFile = new QMenu(menubar);
        menuFile->setObjectName(QString::fromUtf8("menuFile"));
        OverlapMainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(OverlapMainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        OverlapMainWindow->setStatusBar(statusbar);

        menubar->addAction(menuFile->menuAction());
        menuFile->addAction(actionQuit);

        retranslateUi(OverlapMainWindow);
        QObject::connect(actionQuit, SIGNAL(activated()), OverlapMainWindow, SLOT(close()));

        QMetaObject::connectSlotsByName(OverlapMainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *OverlapMainWindow)
    {
        OverlapMainWindow->setWindowTitle(QApplication::translate("OverlapMainWindow", "MainWindow", 0, QApplication::UnicodeUTF8));
        actionQuit->setText(QApplication::translate("OverlapMainWindow", "&Quit", 0, QApplication::UnicodeUTF8));
        groupBox->setTitle(QApplication::translate("OverlapMainWindow", "Selected features", 0, QApplication::UnicodeUTF8));
        groupBox_2->setTitle(QApplication::translate("OverlapMainWindow", "Overlapping Features", 0, QApplication::UnicodeUTF8));
        menuFile->setTitle(QApplication::translate("OverlapMainWindow", "File", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class OverlapMainWindow: public Ui_OverlapMainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_OVERLAPVIEWER_H
