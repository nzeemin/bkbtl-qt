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

enum DisasmLineType
{
    LINETYPE_NONE     = 0,  // Empty line
    LINETYPE_DATA     = 1,  // Line contains a data (non-instruction)
    LINETYPE_INSTR    = 2,  // Line contains a disassembled instruction
    LINETYPE_JUMP     = 4,  // Line has jump
    LINETYPE_SUBTITLE = 8,  // Line has subtitle comment
};

struct DisasmLineItem
{
    int     type;           // Combination of DisasmLineType values
    quint16 address;        // Line address for LINETYPE_DATA
    int     addrtype;       // Address type for LINETYPE_DATA, see ADDRTYPE_XXX constants
    quint16 value;          // Data on the address for LINETYPE_DATA
    char    strInstr[8];    // Disassembled instruction for LINETYPE_DISASM
    char    strArg[32];     // Disassembled instruction arguments for LINETYPE_DISASM
    int     jumpdelta;      // Jump delta for LINETYPE_JUMP
    const DisasmSubtitleItem * pSubItem;  // Link to subtitles item for LINETYPE_SUBTITLE
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
    void paintEvent(QPaintEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void focusInEvent(QFocusEvent *) override;
    void focusOutEvent(QFocusEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void mousePressEvent(QMouseEvent *) override;

    void parseSubtitles(QTextStream& stream);
    void addSubtitle(quint16 addr, DisasmSubtitleType type, const QString& comment);

private:
    quint16 m_wDisasmBaseAddr;
    quint16 m_wDisasmNextBaseAddr;
    QVector<DisasmSubtitleItem> m_SubtitleItems;
    bool m_okDisasmJumpPredict;
    char m_strDisasmHint[42];
    char m_strDisasmHint2[42];
    QVector<DisasmLineItem> m_DisasmLineItems;
    int m_cxDisasmBreakpointZone;
    int m_cyDisasmLine;

    const DisasmSubtitleItem * findSubtitle(quint16 address, quint16 typemask);

    void drawJump(QPainter& painter, int yFrom, int delta, int x, int cyLine, QColor color);
    void drawBreakpoint(QPainter& painter, int x, int y, int cyLine);
    int drawDisassemble(QPainter& painter, CProcessor* pProc, quint16 current, quint16 previous);
};

#endif // QDISASMVIEW_H
