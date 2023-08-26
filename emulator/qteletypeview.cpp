#include "stdafx.h"
#include <QTextEdit>
#include <QVBoxLayout>
#include "qteletypeview.h"

QTeletypeView::QTeletypeView()
{
    setMinimumSize(320, 180);

    m_log = new QTextEdit(this);
    QFont font = Common_GetMonospacedFont();
    m_log->setReadOnly(true);
    m_log->setFont(font);
    m_log->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    QVBoxLayout *vboxlayout = new QVBoxLayout;
    vboxlayout->setMargin(0);
    vboxlayout->setSpacing(4);
    vboxlayout->addWidget(m_log);
    this->setLayout(vboxlayout);
}

QTeletypeView::~QTeletypeView()
{
    delete m_log;
}

void QTeletypeView::print(const QString & message)
{
    // Put selection to the end of text
    m_log->moveCursor(QTextCursor::End);
    // Insert the message
    m_log->insertPlainText(message);
    //TODO: Scroll to caret
}
