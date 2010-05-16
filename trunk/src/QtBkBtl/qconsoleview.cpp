#include "stdafx.h"
#include "qconsoleview.h"
#include <QLineEdit>
#include <QTextEdit>
#include <QVBoxLayout>

QConsoleView::QConsoleView()
{
    setMinimumSize(320, 180);

    m_log = new QTextEdit();
    m_edit = new QLineEdit();

    QVBoxLayout *vboxlayout = new QVBoxLayout;
    vboxlayout->setMargin(0);
    vboxlayout->setSpacing(4);
    vboxlayout->addWidget(m_log);
    vboxlayout->addWidget(m_edit);
    this->setLayout(vboxlayout);

    QFont font = Common_GetMonospacedFont();
    m_log->setReadOnly(true);
    m_log->setFont(font);
    m_edit->setFont(font);
}
