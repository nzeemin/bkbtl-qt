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
    Q_PROPERTY(ushort sp READ getSP WRITE setSP)
    /// \brief Get the processor PC register value, short form for getPC().
    Q_PROPERTY(ushort pc READ getPC WRITE setPC)
    /// \brief Get the processor status word value, short form for getPSW().
    Q_PROPERTY(ushort psw READ getPSW WRITE setPSW)

public:
    QEmulatorProcessor(CProcessor* processor);

public slots:
    /// \brief Get the processor register value.
    /// \param regno 0..7
    ushort getReg(int regno);
    /// \brief Get the processor register value, short form for getReg().
    /// \param regno 0..7
    ushort r(int regno) { return getReg(regno); }
    /// \brief Get the processor stack register value.
    ushort getSP() { return getReg(6); }
    /// \brief Get the processor PC register value.
    ushort getPC() { return getReg(7); }
    /// \brief Get the processor status word value.
    ushort getPSW();

    /// \brief Put the given value to the given processor register.
    /// \param regno 0..7
    /// \param value Value to put in the processor register.
    void setReg(int regno, ushort value);
    /// \brief Set the processor SP register value.
    void setSP(ushort value) { setReg(6, value); }
    /// \brief Set the processor PC register value.
    void setPC(ushort value) { setReg(7, value); }
    /// \brief Set the processor PSW register value.
    void setPSW(ushort value);

private:
    CProcessor* m_processor;
};

class QEmulator : public QObject
{
    Q_OBJECT

    /// \brief Get emulator uptime, in seconds, short for for getUptime().
    Q_PROPERTY(float uptime READ getUptime)
    /// \brief Get CPU object, short form for getCPU().
    Q_PROPERTY(QObject* cpu READ getCPU)

public:
    QEmulator(QScriptWindow * window, QScriptEngine * engine);

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
    void saveScreenshot(const QString& filename);

    /// \brief Get CPU object.
    /// \return QEmulatorProcessor object.
    QObject* getCPU() { return &m_cpu; }

    /// \brief Read word from the processor memory.
    /// \param addr memory address
    ushort readWord(ushort addr);
    /// \brief Read byte from the processor memory.
    /// \param addr memory address
    uchar readByte(ushort addr);

    /// \brief Disassemble one instruction at the given address.
    /// \param addr memory address
    /// \return Array of four: { address, instruction, arguments, instruction length }.
    QScriptValue disassemble(ushort addr);

    /// \brief Press the key (by scan code), wait timeout frames, release the key, wait 3 frames.
    /// \param ukncscan BK scan code
    void keyScan(uchar bkscan, int timeout = 3);
    /// \brief Type the key sequence.
    void keyString(const QString& str);

    /// \brief Print a message to debug console window.
    /// \param message The message to print.
    void consolePrint(const QString& message);

    /// \brief Load .BIN-file into the memory.
    /// \param binfilename Path/name of the .bin file to load.
    bool loadBin(const QString& binfilename);

    //TODO: Configurations
    //TODO: Disks
    //TODO: Change screen mode

private:
    void keyChar(char ch, int timeout = 3);

private:
    QScriptWindow * m_window;
    QScriptEngine * m_engine;
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
