#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QEmulatorScreen;
class QKeyboardView;
class QConsoleView;
class QDebugView;
class QDisasmView;
class QMemoryView;
class QTeletypeView;
class QTapeView;
class QLabel;
class QSettings;

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

public:
    void UpdateMenu();
    void UpdateAllViews();
    void saveSettings(QSettings *);
    void restoreSettings(QSettings *);
    void printToTeletype(const QString&);
    void showUptime(int uptimeMillisec);
    void showFps(double framesPerSecond);

public:
    void saveStateImage(const QString& filename);
    void loadStateImage(const QString& filename);
public slots:
    void saveStateImage();
    void loadStateImage();
    void fileLoadBin();
    void saveScreenshot();
    void saveScreenshot(const QString&);
    void helpAbout();
    void emulatorFrame();
    void emulatorRun();
    void emulatorReset();
    void emulatorColorScreen();
    void emulatorScreenMode(int mode);
    void configurationBK0010Basic();
    void configurationBK0010Focal();
    void configurationBK0010Fdd();
    void configurationBK0011();
    void configurationBK0011Fdd();
    void emulatorFloppy(int slot);
    void debugConsoleView();
    void debugDebugView();
    void debugDisasmView();
    void debugMemoryView();
    void debugTeletypeView();
    void debugStepInto();
    void debugStepOver();
    void scriptRun();
    void consolePrint(const QString&);

protected:
    void changeEvent(QEvent *);
    void closeEvent(QCloseEvent *);

private:
    Ui::MainWindow *ui;

    QEmulatorScreen *m_screen;
    QKeyboardView *m_keyboard;
    QConsoleView *m_console;
    QDockWidget* m_dockConsole;
    QDebugView *m_debug;
    QDockWidget* m_dockDebug;
    QDisasmView *m_disasm;
    QDockWidget* m_dockDisasm;
    QMemoryView * m_memory;
    QDockWidget* m_dockMemory;
    QTeletypeView * m_teletype;
    QDockWidget* m_dockTeletype;
    QTapeView * m_tape;
    QDockWidget* m_dockTape;

    QLabel* m_statusLabelInfo;
    QLabel* m_statusLabelFrames;
    QLabel* m_statusLabelUptime;

    void setConfiguration(int configuration);

};

#endif // MAINWINDOW_H
