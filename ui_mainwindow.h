/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created: Sun May 16 21:56:29 2010
**      by: Qt User Interface Compiler version 4.6.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QStatusBar>
#include <QtGui/QToolBar>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionFileExit;
    QAction *actionEmulatorRun;
    QAction *actionEmulatorReset;
    QAction *actionHelpAboutQt;
    QAction *actionDrivesFloppy0;
    QAction *actionDrivesFloppy1;
    QAction *actionDrivesFloppy2;
    QAction *actionDrivesFloppy3;
    QAction *actionDrivesCartridge1;
    QAction *actionDrivesHard1;
    QAction *actionDrivesCartridge2;
    QAction *actionDrivesHard2;
    QAction *actionFileScreenshot;
    QAction *actionConfBK10Basic;
    QAction *actionConfBK10Focal;
    QAction *actionConfBK10Fdd;
    QAction *actionConfBK11;
    QAction *actionConfBK11Fdd;
    QAction *actionEmulatorColorScreen;
    QAction *actionFileLoadBin;
    QAction *actionDebugDebugView;
    QAction *actionDebugDisasmView;
    QAction *actionDebugMemoryView;
    QAction *actionDebugConsoleView;
    QAction *actionDebugStepInto;
    QAction *actionDebugStepOver;
    QWidget *centralWidget;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QMenu *menuEmulator;
    QMenu *menuDrives;
    QMenu *menuHelp;
    QMenu *menuConfiguration;
    QMenu *menuDebug;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(662, 571);
        actionFileExit = new QAction(MainWindow);
        actionFileExit->setObjectName(QString::fromUtf8("actionFileExit"));
        actionEmulatorRun = new QAction(MainWindow);
        actionEmulatorRun->setObjectName(QString::fromUtf8("actionEmulatorRun"));
        actionEmulatorRun->setCheckable(true);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/images/iconRun.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionEmulatorRun->setIcon(icon);
        actionEmulatorReset = new QAction(MainWindow);
        actionEmulatorReset->setObjectName(QString::fromUtf8("actionEmulatorReset"));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/images/iconReset.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionEmulatorReset->setIcon(icon1);
        actionHelpAboutQt = new QAction(MainWindow);
        actionHelpAboutQt->setObjectName(QString::fromUtf8("actionHelpAboutQt"));
        actionDrivesFloppy0 = new QAction(MainWindow);
        actionDrivesFloppy0->setObjectName(QString::fromUtf8("actionDrivesFloppy0"));
        actionDrivesFloppy0->setCheckable(false);
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/images/iconFloppySlot.png"), QSize(), QIcon::Normal, QIcon::Off);
        icon2.addFile(QString::fromUtf8(":/images/iconFloppy.png"), QSize(), QIcon::Normal, QIcon::On);
        actionDrivesFloppy0->setIcon(icon2);
        actionDrivesFloppy1 = new QAction(MainWindow);
        actionDrivesFloppy1->setObjectName(QString::fromUtf8("actionDrivesFloppy1"));
        actionDrivesFloppy1->setCheckable(false);
        actionDrivesFloppy1->setIcon(icon2);
        actionDrivesFloppy2 = new QAction(MainWindow);
        actionDrivesFloppy2->setObjectName(QString::fromUtf8("actionDrivesFloppy2"));
        actionDrivesFloppy2->setCheckable(false);
        actionDrivesFloppy2->setIcon(icon2);
        actionDrivesFloppy3 = new QAction(MainWindow);
        actionDrivesFloppy3->setObjectName(QString::fromUtf8("actionDrivesFloppy3"));
        actionDrivesFloppy3->setCheckable(false);
        actionDrivesFloppy3->setIcon(icon2);
        actionDrivesCartridge1 = new QAction(MainWindow);
        actionDrivesCartridge1->setObjectName(QString::fromUtf8("actionDrivesCartridge1"));
        actionDrivesCartridge1->setCheckable(false);
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/images/iconCartridgeSlot.png"), QSize(), QIcon::Normal, QIcon::Off);
        icon3.addFile(QString::fromUtf8(":/images/iconCartridge.png"), QSize(), QIcon::Normal, QIcon::On);
        actionDrivesCartridge1->setIcon(icon3);
        actionDrivesHard1 = new QAction(MainWindow);
        actionDrivesHard1->setObjectName(QString::fromUtf8("actionDrivesHard1"));
        actionDrivesHard1->setCheckable(false);
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/images/iconHddSlot.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionDrivesHard1->setIcon(icon4);
        actionDrivesCartridge2 = new QAction(MainWindow);
        actionDrivesCartridge2->setObjectName(QString::fromUtf8("actionDrivesCartridge2"));
        actionDrivesCartridge2->setCheckable(false);
        actionDrivesCartridge2->setIcon(icon3);
        actionDrivesHard2 = new QAction(MainWindow);
        actionDrivesHard2->setObjectName(QString::fromUtf8("actionDrivesHard2"));
        actionDrivesHard2->setCheckable(false);
        actionDrivesHard2->setIcon(icon4);
        actionFileScreenshot = new QAction(MainWindow);
        actionFileScreenshot->setObjectName(QString::fromUtf8("actionFileScreenshot"));
        actionConfBK10Basic = new QAction(MainWindow);
        actionConfBK10Basic->setObjectName(QString::fromUtf8("actionConfBK10Basic"));
        actionConfBK10Basic->setCheckable(true);
        actionConfBK10Focal = new QAction(MainWindow);
        actionConfBK10Focal->setObjectName(QString::fromUtf8("actionConfBK10Focal"));
        actionConfBK10Focal->setCheckable(true);
        actionConfBK10Fdd = new QAction(MainWindow);
        actionConfBK10Fdd->setObjectName(QString::fromUtf8("actionConfBK10Fdd"));
        actionConfBK10Fdd->setCheckable(true);
        actionConfBK11 = new QAction(MainWindow);
        actionConfBK11->setObjectName(QString::fromUtf8("actionConfBK11"));
        actionConfBK11->setCheckable(true);
        actionConfBK11Fdd = new QAction(MainWindow);
        actionConfBK11Fdd->setObjectName(QString::fromUtf8("actionConfBK11Fdd"));
        actionConfBK11Fdd->setCheckable(true);
        actionEmulatorColorScreen = new QAction(MainWindow);
        actionEmulatorColorScreen->setObjectName(QString::fromUtf8("actionEmulatorColorScreen"));
        actionFileLoadBin = new QAction(MainWindow);
        actionFileLoadBin->setObjectName(QString::fromUtf8("actionFileLoadBin"));
        actionDebugDebugView = new QAction(MainWindow);
        actionDebugDebugView->setObjectName(QString::fromUtf8("actionDebugDebugView"));
        actionDebugDebugView->setCheckable(true);
        actionDebugDisasmView = new QAction(MainWindow);
        actionDebugDisasmView->setObjectName(QString::fromUtf8("actionDebugDisasmView"));
        actionDebugDisasmView->setCheckable(true);
        actionDebugMemoryView = new QAction(MainWindow);
        actionDebugMemoryView->setObjectName(QString::fromUtf8("actionDebugMemoryView"));
        actionDebugMemoryView->setCheckable(true);
        actionDebugConsoleView = new QAction(MainWindow);
        actionDebugConsoleView->setObjectName(QString::fromUtf8("actionDebugConsoleView"));
        actionDebugConsoleView->setCheckable(true);
        actionDebugStepInto = new QAction(MainWindow);
        actionDebugStepInto->setObjectName(QString::fromUtf8("actionDebugStepInto"));
        actionDebugStepOver = new QAction(MainWindow);
        actionDebugStepOver->setObjectName(QString::fromUtf8("actionDebugStepOver"));
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 662, 21));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QString::fromUtf8("menuFile"));
        menuEmulator = new QMenu(menuBar);
        menuEmulator->setObjectName(QString::fromUtf8("menuEmulator"));
        menuDrives = new QMenu(menuBar);
        menuDrives->setObjectName(QString::fromUtf8("menuDrives"));
        menuDrives->setEnabled(false);
        menuHelp = new QMenu(menuBar);
        menuHelp->setObjectName(QString::fromUtf8("menuHelp"));
        menuConfiguration = new QMenu(menuBar);
        menuConfiguration->setObjectName(QString::fromUtf8("menuConfiguration"));
        menuDebug = new QMenu(menuBar);
        menuDebug->setObjectName(QString::fromUtf8("menuDebug"));
        MainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        mainToolBar->setMovable(false);
        mainToolBar->setAllowedAreas(Qt::TopToolBarArea);
        mainToolBar->setIconSize(QSize(16, 16));
        mainToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        mainToolBar->setFloatable(false);
        MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        MainWindow->setStatusBar(statusBar);

        menuBar->addAction(menuFile->menuAction());
        menuBar->addAction(menuEmulator->menuAction());
        menuBar->addAction(menuConfiguration->menuAction());
        menuBar->addAction(menuDrives->menuAction());
        menuBar->addAction(menuDebug->menuAction());
        menuBar->addAction(menuHelp->menuAction());
        menuFile->addAction(actionFileLoadBin);
        menuFile->addSeparator();
        menuFile->addAction(actionFileScreenshot);
        menuFile->addSeparator();
        menuFile->addAction(actionFileExit);
        menuEmulator->addAction(actionEmulatorRun);
        menuEmulator->addAction(actionEmulatorReset);
        menuEmulator->addSeparator();
        menuEmulator->addAction(actionEmulatorColorScreen);
        menuDrives->addAction(actionDrivesFloppy0);
        menuDrives->addAction(actionDrivesFloppy1);
        menuDrives->addAction(actionDrivesFloppy2);
        menuDrives->addAction(actionDrivesFloppy3);
        menuHelp->addAction(actionHelpAboutQt);
        menuConfiguration->addAction(actionConfBK10Basic);
        menuConfiguration->addAction(actionConfBK10Focal);
        menuConfiguration->addAction(actionConfBK10Fdd);
        menuConfiguration->addAction(actionConfBK11);
        menuConfiguration->addAction(actionConfBK11Fdd);
        menuDebug->addAction(actionDebugConsoleView);
        menuDebug->addSeparator();
        menuDebug->addAction(actionDebugDebugView);
        menuDebug->addAction(actionDebugDisasmView);
        menuDebug->addAction(actionDebugMemoryView);
        menuDebug->addSeparator();
        menuDebug->addAction(actionDebugStepInto);
        menuDebug->addAction(actionDebugStepOver);
        mainToolBar->addAction(actionEmulatorRun);
        mainToolBar->addAction(actionEmulatorReset);
        mainToolBar->addSeparator();
        mainToolBar->addAction(actionEmulatorColorScreen);
        mainToolBar->addSeparator();

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0, QApplication::UnicodeUTF8));
        actionFileExit->setText(QApplication::translate("MainWindow", "Exit", 0, QApplication::UnicodeUTF8));
        actionEmulatorRun->setText(QApplication::translate("MainWindow", "Run", 0, QApplication::UnicodeUTF8));
        actionEmulatorReset->setText(QApplication::translate("MainWindow", "Reset", 0, QApplication::UnicodeUTF8));
        actionHelpAboutQt->setText(QApplication::translate("MainWindow", "About Qt", 0, QApplication::UnicodeUTF8));
        actionDrivesFloppy0->setText(QApplication::translate("MainWindow", "Floppy MZ0:", 0, QApplication::UnicodeUTF8));
        actionDrivesFloppy0->setIconText(QApplication::translate("MainWindow", "0", 0, QApplication::UnicodeUTF8));
        actionDrivesFloppy1->setText(QApplication::translate("MainWindow", "Floppy MZ1:", 0, QApplication::UnicodeUTF8));
        actionDrivesFloppy1->setIconText(QApplication::translate("MainWindow", "1", 0, QApplication::UnicodeUTF8));
        actionDrivesFloppy2->setText(QApplication::translate("MainWindow", "Floppy MZ2:", 0, QApplication::UnicodeUTF8));
        actionDrivesFloppy2->setIconText(QApplication::translate("MainWindow", "2", 0, QApplication::UnicodeUTF8));
        actionDrivesFloppy3->setText(QApplication::translate("MainWindow", "Floppy MZ3:", 0, QApplication::UnicodeUTF8));
        actionDrivesFloppy3->setIconText(QApplication::translate("MainWindow", "3", 0, QApplication::UnicodeUTF8));
        actionDrivesCartridge1->setText(QApplication::translate("MainWindow", "Cartridge 1", 0, QApplication::UnicodeUTF8));
        actionDrivesCartridge1->setIconText(QApplication::translate("MainWindow", "1", 0, QApplication::UnicodeUTF8));
        actionDrivesHard1->setText(QApplication::translate("MainWindow", "Hard Drive 1", 0, QApplication::UnicodeUTF8));
        actionDrivesHard1->setIconText(QApplication::translate("MainWindow", "1", 0, QApplication::UnicodeUTF8));
        actionDrivesCartridge2->setText(QApplication::translate("MainWindow", "Cartridge 2", 0, QApplication::UnicodeUTF8));
        actionDrivesCartridge2->setIconText(QApplication::translate("MainWindow", "2", 0, QApplication::UnicodeUTF8));
        actionDrivesHard2->setText(QApplication::translate("MainWindow", "Hard Drive 2", 0, QApplication::UnicodeUTF8));
        actionDrivesHard2->setIconText(QApplication::translate("MainWindow", "2", 0, QApplication::UnicodeUTF8));
        actionFileScreenshot->setText(QApplication::translate("MainWindow", "Screenshot", 0, QApplication::UnicodeUTF8));
        actionConfBK10Basic->setText(QApplication::translate("MainWindow", "BK 0010.01 BASIC", 0, QApplication::UnicodeUTF8));
        actionConfBK10Focal->setText(QApplication::translate("MainWindow", "BK 0010.01 FOCAL", 0, QApplication::UnicodeUTF8));
        actionConfBK10Fdd->setText(QApplication::translate("MainWindow", "BK 0010.01 FDD", 0, QApplication::UnicodeUTF8));
        actionConfBK11->setText(QApplication::translate("MainWindow", "BK 0011M", 0, QApplication::UnicodeUTF8));
        actionConfBK11Fdd->setText(QApplication::translate("MainWindow", "BK 0011M FDD", 0, QApplication::UnicodeUTF8));
        actionEmulatorColorScreen->setText(QApplication::translate("MainWindow", "Color Screen", 0, QApplication::UnicodeUTF8));
        actionEmulatorColorScreen->setIconText(QApplication::translate("MainWindow", "Color", 0, QApplication::UnicodeUTF8));
        actionFileLoadBin->setText(QApplication::translate("MainWindow", "Load BIN", 0, QApplication::UnicodeUTF8));
        actionDebugDebugView->setText(QApplication::translate("MainWindow", "Processor View", 0, QApplication::UnicodeUTF8));
        actionDebugDisasmView->setText(QApplication::translate("MainWindow", "Disasm View", 0, QApplication::UnicodeUTF8));
        actionDebugMemoryView->setText(QApplication::translate("MainWindow", "Memory View", 0, QApplication::UnicodeUTF8));
        actionDebugConsoleView->setText(QApplication::translate("MainWindow", "Debug Views", 0, QApplication::UnicodeUTF8));
        actionDebugStepInto->setText(QApplication::translate("MainWindow", "Step Into", 0, QApplication::UnicodeUTF8));
        actionDebugStepInto->setShortcut(QApplication::translate("MainWindow", "F11", 0, QApplication::UnicodeUTF8));
        actionDebugStepOver->setText(QApplication::translate("MainWindow", "Step Over", 0, QApplication::UnicodeUTF8));
        actionDebugStepOver->setShortcut(QApplication::translate("MainWindow", "F10", 0, QApplication::UnicodeUTF8));
        menuFile->setTitle(QApplication::translate("MainWindow", "File", 0, QApplication::UnicodeUTF8));
        menuEmulator->setTitle(QApplication::translate("MainWindow", "Emulator", 0, QApplication::UnicodeUTF8));
        menuDrives->setTitle(QApplication::translate("MainWindow", "Drives", 0, QApplication::UnicodeUTF8));
        menuHelp->setTitle(QApplication::translate("MainWindow", "Help", 0, QApplication::UnicodeUTF8));
        menuConfiguration->setTitle(QApplication::translate("MainWindow", "Configuration", 0, QApplication::UnicodeUTF8));
        menuDebug->setTitle(QApplication::translate("MainWindow", "Debug", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
