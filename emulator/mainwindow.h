#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QElapsedTimer>

class QEmulatorScreen;
class QKeyboardView;
class QConsoleView;
class QDebugView;
class QDisasmView;
class QInstrHistoryView;
class QMemoryView;
class QTeletypeView;
class QTapeView;
class QLabel;
class QSettings;
class QVBoxLayout;

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public:
    void updateMenu();
    void updateAllViews();
    void redrawDebugView();
    void redrawDisasmView();
    void updateWindowText();
    void saveSettings(QSettings *);
    void restoreSettings(QSettings *);
    void printToTeletype(const QString&);
    void showUptime(int uptimeMillisec);
    void showFps(double framesPerSecond);
    bool isDebugMode();
    void focusScreen();

public:
    void saveStateImage(const QString& filename);
    void loadStateImage(const QString& filename);
    void saveScreenshot(const QString& filename);

public slots:
    void saveStateImage();
    void loadStateImage();
    void fileLoadBin();
    void saveScreenshot();
    void saveScreenshotAs();
    void screenshotToClipboard();
    void helpAbout();
    void emulatorFrame();
    void emulatorRun();
    void emulatorReset();
    void emulatorAutostart();
    void emulatorColorScreen();
    void emulatorScreenMode(int mode);
    void configurationBK0010Basic();
    void configurationBK0010Focal();
    void configurationBK0010Fdd();
    void configurationBK0011();
    void configurationBK0011Fdd();
    void configurationVM1G();
    void emulatorFloppy(int slot);
    void debugConsoleView();
    void debugDisasmView();
    void debugInstrHistoryView();
    void debugMemoryView();
    void debugTeletypeView();
    void debugStepInto();
    void debugStepOver();
    void debugClearConsole();
    void debugRemoveAllBreakpoints();
    void viewKeyboard();
    void viewTape();
    void soundEnabled();
    void scriptRun();
    void consolePrint(const QString&);

protected:
    void showEvent(QShowEvent *);
    void changeEvent(QEvent *);
    void closeEvent(QCloseEvent *);
    bool eventFilter(QObject *watched, QEvent *event);

private:
    Ui::MainWindow *ui;
    bool autoStartProcessed;

    QVBoxLayout* m_centralLayout;
    QEmulatorScreen *m_screen;
    QKeyboardView *m_keyboard;
    QConsoleView *m_console;
    QDockWidget* m_dockConsole;
    QDebugView *m_debug;
    QDockWidget* m_dockDebug;
    QDisasmView *m_disasm;
    QDockWidget* m_dockDisasm;
    QInstrHistoryView *m_instrHistory;
    QDockWidget* m_dockInstrHistory;
    QMemoryView * m_memory;
    QDockWidget* m_dockMemory;
    QTeletypeView * m_teletype;
    QDockWidget* m_dockTeletype;
    QTapeView * m_tape;
    QDockWidget* m_dockTape;

    QLabel* m_statusLabelInfo;
    QLabel* m_statusLabelFrames;
    QLabel* m_statusLabelUptime;

    QElapsedTimer m_frameElapsed;  // Wall-clock pacing for emulatorFrame()
    double m_frameLeftover;        // Accumulated milliseconds not yet turned into frames

    void setConfiguration(int configuration);
    void updateViewsLayout();
};

#endif // MAINWINDOW_H
