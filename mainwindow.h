#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QScreen;
class QKeyboardView;
class QDebugView;

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

protected:
    void changeEvent(QEvent *e);

private:
    Ui::MainWindow *ui;
    QScreen *m_screen;
    QKeyboardView *m_keyboard;
    QDebugView *m_debug;

    void setConfiguration(int configuration);
    void emulatorFloppy(int slot);
};

#endif // MAINWINDOW_H
