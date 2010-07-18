#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QScreen;
class QKeyboardView;
class QConsoleView;
class QDebugView;
class QDisasmView;
class QMemoryView;
class QTeletypeView;
class QLabel;

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

public:
    void UpdateMenu();
    void UpdateAllViews();
    void restoreSettings();
    void printToTeletype(const QString&);
    void showUptime(int uptimeMillisec);
    void showFps(double framesPerSecond);

public slots:
    void fileLoadBin();
    void fileScreenshot();
    void helpAboutQt();
    void emulatorFrame();
    void emulatorRun();
    void emulatorReset();
    void emulatorColorScreen();
    void configurationBK0010Basic();
    void configurationBK0010Focal();
    void configurationBK0010Fdd();
    void configurationBK0011();
    void configurationBK0011Fdd();
    void emulatorFloppy0();
    void emulatorFloppy1();
    void emulatorFloppy2();
    void emulatorFloppy3();
    void debugConsoleView();
    void debugDebugView();
    void debugDisasmView();
    void debugMemoryView();
    void debugTeletypeView();
    void debugStepInto();
    void debugStepOver();

protected:
    void changeEvent(QEvent *);
    void closeEvent(QCloseEvent *);

private:
    Ui::MainWindow *ui;

    QScreen *m_screen;
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

    QLabel* m_statusLabelInfo;
    QLabel* m_statusLabelFrames;
    QLabel* m_statusLabelUptime;

    void setConfiguration(int configuration);
    void emulatorFloppy(int slot);
};

#endif // MAINWINDOW_H
