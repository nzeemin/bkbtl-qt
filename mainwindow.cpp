#include "stdafx.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QAction>
#include <QVBoxLayout>
#include "main.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qscreen.h"
#include "qkeyboardview.h"
#include "Emulator.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setWindowTitle(_T("BK Back to Life"));

    // Assign signals
    QObject::connect(ui->actionFileScreenshot, SIGNAL(triggered()), this, SLOT(fileScreenshot()));
    QObject::connect(ui->actionFileExit, SIGNAL(triggered()), this, SLOT(close()));
    QObject::connect(ui->actionEmulatorRun, SIGNAL(triggered()), this, SLOT(emulatorRun()));
    QObject::connect(ui->actionEmulatorReset, SIGNAL(triggered()), this, SLOT(emulatorReset()));
    QObject::connect(ui->actionEmulatorColorScreen, SIGNAL(triggered()), this, SLOT(emulatorColorScreen()));
    QObject::connect(ui->actionConfBK10Basic, SIGNAL(triggered()), this, SLOT(configurationBK0010Basic()));
    QObject::connect(ui->actionConfBK10Focal, SIGNAL(triggered()), this, SLOT(configurationBK0010Focal()));
    QObject::connect(ui->actionConfBK10Fdd, SIGNAL(triggered()), this, SLOT(configurationBK0010Fdd()));
    QObject::connect(ui->actionConfBK11, SIGNAL(triggered()), this, SLOT(configurationBK0011()));
    QObject::connect(ui->actionConfBK11Fdd, SIGNAL(triggered()), this, SLOT(configurationBK0011Fdd()));
    //QObject::connect(ui->actionDrivesFloppy0, SIGNAL(triggered()), this, SLOT(emulatorFloppy0()));
    //QObject::connect(ui->actionDrivesFloppy1, SIGNAL(triggered()), this, SLOT(emulatorFloppy1()));
    //QObject::connect(ui->actionDrivesFloppy2, SIGNAL(triggered()), this, SLOT(emulatorFloppy2()));
    //QObject::connect(ui->actionDrivesFloppy3, SIGNAL(triggered()), this, SLOT(emulatorFloppy3()));
    QObject::connect(ui->actionHelpAboutQt, SIGNAL(triggered()), this, SLOT(helpAboutQt()));

    // Screen and keyboard
    m_screen = new QScreen();
    m_keyboard = new QKeyboardView();
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(4);
    layout->setSpacing(4);
    layout->addWidget(m_screen);
    layout->addWidget(m_keyboard);
    ui->centralWidget->setLayout(layout);
    this->adjustSize();
    this->setFocusProxy(m_screen);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_screen;
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::UpdateMenu()
{
    ui->actionConfBK10Basic->setChecked(g_nEmulatorConfiguration == BK_CONF_BK0010_BASIC);
    ui->actionConfBK10Focal->setChecked(g_nEmulatorConfiguration == BK_CONF_BK0010_FOCAL);
    ui->actionConfBK10Fdd->setChecked(g_nEmulatorConfiguration == BK_CONF_BK0010_FDD);
    ui->actionConfBK11->setChecked(g_nEmulatorConfiguration == BK_CONF_BK0011);
    ui->actionConfBK11Fdd->setChecked(g_nEmulatorConfiguration == BK_CONF_BK0011_FDD);

    ui->actionDrivesFloppy0->setIcon(QIcon(
            g_pBoard->IsFloppyImageAttached(0) ? _T(":/images/iconFloppy.png") : _T(":/images/iconFloppySlot.png") ));
    ui->actionDrivesFloppy1->setIcon(QIcon(
            g_pBoard->IsFloppyImageAttached(1) ? _T(":/images/iconFloppy.png") : _T(":/images/iconFloppySlot.png") ));
    ui->actionDrivesFloppy2->setIcon(QIcon(
            g_pBoard->IsFloppyImageAttached(2) ? _T(":/images/iconFloppy.png") : _T(":/images/iconFloppySlot.png") ));
    ui->actionDrivesFloppy3->setIcon(QIcon(
            g_pBoard->IsFloppyImageAttached(3) ? _T(":/images/iconFloppy.png") : _T(":/images/iconFloppySlot.png") ));
}

void MainWindow::fileScreenshot()
{
    QFileDialog dlg;
    dlg.setAcceptMode(QFileDialog::AcceptSave);
    dlg.setNameFilter(_T("PNG images (*.png)"));
    if (dlg.exec() == QDialog::Rejected)
        return;

    QString strFileName = dlg.selectedFiles().at(0);

    m_screen->saveScreenshot(strFileName);
}

void MainWindow::helpAboutQt()
{
    QMessageBox::aboutQt(this, _T("About Qt"));
}

void MainWindow::emulatorFrame()
{
    if (g_okEmulatorRunning)
    {
        if (Emulator_SystemFrame())
        {
            m_screen->repaint();
        }
    }
}

void MainWindow::emulatorRun()
{
    if (g_okEmulatorRunning)
    {
        this->setWindowTitle(_T("BK Back to Life"));
        Emulator_Stop();
    }
    else
    {
        this->setWindowTitle(_T("BK Back to Life [run]"));
        Emulator_Start();
    }
}

void MainWindow::emulatorReset()
{
    Emulator_Reset();

    m_screen->repaint();
}

void MainWindow::emulatorColorScreen()
{
    ScreenViewMode newMode = (m_screen->mode() == ColorScreen) ? BlackWhiteScreen : ColorScreen;
    m_screen->setMode(newMode);
}

void MainWindow::configurationBK0010Basic() { setConfiguration(BK_CONF_BK0010_BASIC); }
void MainWindow::configurationBK0010Focal() { setConfiguration(BK_CONF_BK0010_FOCAL); }
void MainWindow::configurationBK0010Fdd()   { setConfiguration(BK_CONF_BK0010_FDD); }
void MainWindow::configurationBK0011()      { setConfiguration(BK_CONF_BK0011); }
void MainWindow::configurationBK0011Fdd()   { setConfiguration(BK_CONF_BK0011_FDD); }
void MainWindow::setConfiguration(int configuration)
{
    // Check if configuration changed
    if (g_nEmulatorConfiguration == configuration)
        return;

    // Ask user -- we have to reset machine to change configuration
    if (!AlertOkCancel(_T("Reset required after configuration change.\nAre you agree?")))
        return;

    // Change configuration
    Emulator_InitConfiguration((BKConfiguration)configuration);

    //Settings_SetConfiguration(configuration);

    UpdateMenu();
    //MainWindow_UpdateAllViews();
}

void MainWindow::emulatorFloppy0() { emulatorFloppy(0); }
void MainWindow::emulatorFloppy1() { emulatorFloppy(1); }
void MainWindow::emulatorFloppy2() { emulatorFloppy(2); }
void MainWindow::emulatorFloppy3() { emulatorFloppy(3); }
void MainWindow::emulatorFloppy(int slot)
{
    //if (g_pBoard->IsFloppyImageAttached(slot))
    //{
    //    g_pBoard->DetachFloppyImage(slot);
    //    Settings_SetFloppyFilePath(slot, NULL);
    //}
    //else
    //{
    //    QFileDialog dlg;
    //    dlg.setNameFilter(_T("BK floppy images (*.dsk *.rtd)"));
    //    if (dlg.exec() == QDialog::Rejected)
    //        return;

    //    QString strFileName = dlg.selectedFiles().at(0);
    //    LPCTSTR sFileName = qPrintable(strFileName);

    //    if (! g_pBoard->AttachFloppyImage(slot, sFileName))
    //    {
    //        AlertWarning(_T("Failed to attach floppy image."));
    //        return;
    //    }

    //    Settings_SetFloppyFilePath(slot, sFileName);
    //}

    //UpdateMenu();
}
