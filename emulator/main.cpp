﻿#include "stdafx.h"
#include <QApplication>
#include <QtCore/QThread>
#include <QtCore/QTimer>
#include <QSettings>
#include <QDesktopWidget>
#include "main.h"
#include "mainwindow.h"
#include "Emulator.h"


void RestoreSettings();


static QApplication *g_Application;
static MainWindow *g_MainWindow;
static QSettings *g_Settings;


void ParseCommandLine(int argc, char *argv[]);

#ifdef Q_OS_WIN
#define OPTIONCHAR '/'
#define OPTIONSTR "/"
#else
#define OPTIONCHAR '-'
#define OPTIONSTR "-"
#endif

const char CommandLineHelp[] =
    "Command line options:\n"
    OPTIONSTR "h " OPTIONSTR "help    Show command line options\n"
    OPTIONSTR "autostart " OPTIONSTR "autostarton    Start emulation on window open\n"
    OPTIONSTR "noautostart " OPTIONSTR "autostartoff    Do not start emulation on window open\n"
    OPTIONSTR "sound " OPTIONSTR "soundon    Turn sound on\n"
    OPTIONSTR "nosound " OPTIONSTR "soundoff    Turn sound off\n";


int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    g_Application = &application;

    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "Back to Life", "BKBTL");
    g_Settings = &settings;

    MainWindow w;
    g_MainWindow = &w;

    ParseCommandLine(argc, argv);  // Override settings by command-line option if needed

    if (!Emulator_Init())
        return 255;
    if (!Emulator_InitConfiguration((BKConfiguration)Global_getSettings()->value("Configuration", BK_CONF_BK0010_BASIC).toInt()))
        return 255;

    w.restoreSettings(Global_getSettings());

    w.show();

    RestoreSettings();
    w.updateMenu();
    w.updateAllViews();

    if (Option_ShowHelp)
    {
        AlertInfo(CommandLineHelp);
    }

    QTimer timerFrame;
    QObject::connect(&timerFrame, SIGNAL(timeout()), &w, SLOT(emulatorFrame()), Qt::AutoConnection);
    timerFrame.start(32);

    int result = application.exec();

    Emulator_Done();

    Global_getSettings()->setValue("Configuration", (int)g_nEmulatorConfiguration);
    settings.sync();

    Common_Cleanup();

    return result;
}

QApplication* Global_getApplication()
{
    return g_Application;
}
MainWindow* Global_getMainWindow()
{
    return g_MainWindow;
}
QSettings* Global_getSettings()
{
    return g_Settings;
}

void Global_UpdateAllViews()
{
    Global_getMainWindow()->updateAllViews();
}
void Global_UpdateMenu()
{
    Global_getMainWindow()->updateMenu();
}
void Global_RedrawDebugView()
{
    Global_getMainWindow()->redrawDebugView();
}
void Global_RedrawDisasmView()
{
    Global_getMainWindow()->redrawDisasmView();
}

void Global_showUptime(int uptimeMillisec)
{
    Global_getMainWindow()->showUptime(uptimeMillisec);
}
void Global_showFps(double framesPerSecond)
{
    Global_getMainWindow()->showFps(framesPerSecond);
}

void RestoreSettings()
{
    // Reattach floppy images
    for (int slot = 0; slot < 4; slot++)
    {
        QString path = Settings_GetFloppyFilePath(slot);
        if (path.length() > 0)
        {
            if (! g_pBoard->AttachFloppyImage(slot, qPrintable(path)))
                Settings_SetFloppyFilePath(slot, nullptr);
        }
    }
}

void ParseCommandLine(int argc, char *argv[])
{
    char** it = argv;
    char** itend = argv + argc;
    while (it != itend)
    {
        const char* param = *it;
        if (param[0] == OPTIONCHAR)
        {
            QString option = QString::fromLocal8Bit(param + 1);
            if (option == "help" || option == "h")
            {
                Option_ShowHelp = true;
            }
            else if (option == "autostart" || option == "autostarton")
            {
                Settings_SetAutostart(true);
            }
            else if (option == "noautostart" || option == "autostartoff")
            {
                Settings_SetAutostart(false);
            }
            else if (option == "sound" || option == "soundon")
            {
                Settings_SetSound(true);
            }
            else if (option == "soundoff" || option == "nosound")
            {
                Settings_SetSound(false);
            }
            //TODO
        }

        ++it;
    }
}
