#ifndef QDISASMVIEW_H
#define QDISASMVIEW_H

#include <QVector>
#include <QTextStream>
#include <QWidget>

class QPainter;
class CProcessor;


enum DisasmSubtitleType
{
    SUBTYPE_NONE = 0,
    SUBTYPE_COMMENT = 1,
    SUBTYPE_BLOCKCOMMENT = 2,
    SUBTYPE_DATA = 4
};

struct DisasmSubtitleItem
{
    quint16 address;
    DisasmSubtitleType type;
    QString comment;
};

class QDisasmView : public QWidget
{
    Q_OBJECT
public:
    QDisasmView();

    void updateData();
    void updateWindowText();

public slots:
    void showHideSubtitles();

protected:
    void paintEvent(QPaintEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
    void focusInEvent(QFocusEvent *);
    void focusOutEvent(QFocusEvent *);

    void parseSubtitles(QTextStream& stream);
    void addSubtitle(quint16 addr, DisasmSubtitleType type, const QString& comment);

private:
    quint16 m_wDisasmBaseAddr;
    quint16 m_wDisasmNextBaseAddr;
    QVector<DisasmSubtitleItem> m_SubtitleItems;

    int drawDisassemble(QPainter& painter, CProcessor* pProc, quint16 base, quint16 previous);
    const DisasmSubtitleItem * findSubtitle(quint16 address, quint16 typemask);

    bool checkForJump(const quint16* memory, int* pDelta);
    bool getJumpConditionHint(const quint16* memory, const CProcessor * pProc, QString &buffer);
    void drawJump(QPainter& painter, int yFrom, int delta, int x, int cyLine, QColor color);
    void registerHint(const CProcessor *pProc,
            QString &hint1, QString& hint2,
            int regnum, int regmod, bool byteword, quint16 indexval);
    void registerHintPC(const CProcessor *pProc,
            QString &hint1, QString& hint2,
            int regmod, bool byteword, quint16 curaddr, quint16 value);
    void instructionHint(const quint16 *memory, const CProcessor *pProc,
            QString& buffer, QString& buffer2,
            int srcreg, int srcmod, int dstreg, int dstmod);
    int getInstructionHint(const quint16* memory, const CProcessor * pProc,
            QString &buffer, QString &buffer2);
};

#endif // QDISASMVIEW_H
