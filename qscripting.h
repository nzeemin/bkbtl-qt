#ifndef QSCRIPTING_H
#define QSCRIPTING_H

#include <QObject>
#include <QDialog>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QScriptEngine>

class QScriptWindow;
class CProcessor;


/// \brief Scripting object for emulated processor.
/// Processor is accessible from script as "emulator.cpu".
/// \sa QEmulator
class QEmulatorProcessor : public QObject
{
    Q_OBJECT

    /// \brief Get the processor stack register value, short form for getSP().
    Q_PROPERTY(ushort sp READ getSP)
    /// \brief Get the processor PC register value, short form for getPC().
    Q_PROPERTY(ushort pc READ getPC)
    /// \brief Get the processor status word value, short form for getPSW().
    Q_PROPERTY(ushort psw READ getPSW)

public:
    QEmulatorProcessor(QScriptEngine* engine, CProcessor* processor);

public slots:
    /// \brief Get the processor register value.
    /// \param regno 0..7 */
    ushort getReg(int regno);
    /// \brief Get the processor register value, short form for getReg().
    /// \param regno 0..7 */
    ushort r(int regno) { return getReg(regno); }
    /// \brief Get the processor stack register value.
    ushort getSP() { return getReg(6); }
    /// \brief Get the processor PC register value.
    ushort getPC() { return getReg(7); }
    /// \brief Get the processor status word value.
    ushort getPSW();

private:
    QScriptEngine* m_engine;
    CProcessor* m_processor;
};

class QEmulator : public QObject
{
    Q_OBJECT

    /// \brief Get CPU object, short form for getCPU().
    Q_PROPERTY(QObject* cpu READ getCPU)

public:
    QEmulator(QScriptWindow * window);

public slots:
    /// \brief Resets the emulator.
    void reset();
    /// \brief Runs the emulator for the given number of frames. One second is 25 frames.
    /// \param frames Number of frames to execute. Every frame is 1/25 second.
    bool run(int frames);
    /// \brief Get emulator uptime, in seconds.
    float getUptime();

    /// \brief Sets CPU breakpoint address for the next run() call.
    /// \param address Address of the CPU breakpoint to set.
    void setBreakpoint(quint16 address);
    /// \brief Check if the emulator stopped on a breakpoint.
    bool isBreakpoint();

    /// \brief Save screenshot to PNG file.
    void saveScreenshot(QString filename);

    /// \brief Get CPU object.
    /// \return QEmulatorProcessor object.
    QObject* getCPU() { return &m_cpu; }

    //TODO: Configurations
    //TODO: Disks
    //TODO: Change screen mode

private:
    QScriptWindow * m_window;
    QEmulatorProcessor m_cpu;
};


class QScriptWindow : public QDialog
{
    Q_OBJECT

public:
    QScriptWindow(QWidget * parent = 0);
    ~QScriptWindow();

public:
    void runScript(const QString & script);
    bool isAborted() const { return m_aborted; }
    QScriptEngine* getEngine() { return &m_engine; }

public slots:
    void reject();

private slots:
    void cancelButtonPressed();

private:
    QVBoxLayout m_vlayout;
    QLabel m_static;
    QPushButton m_cancelButton;
    QScriptEngine m_engine;
    bool m_aborted;
    QEmulator * m_emulator;
};


#endif //QSCRIPTING_H
