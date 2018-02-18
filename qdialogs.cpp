#include "stdafx.h"
#include "qdialogs.h"
#include <QMessageBox>

QInputOctalDialog::QInputOctalDialog(QWidget *parent, const QString & title, const QString & prompt, quint16 * value)
    : QDialog(parent, 0)
{
    m_result = value;

    char buffer[8];
    PrintOctalValue(buffer, *value);

    setWindowTitle(title);
    resize(340, 120);
    m_label.setText(prompt);
    m_layout.addWidget(&m_label);
    m_edit.setText(buffer);
    m_edit.selectAll();
    m_layout.addWidget(&m_edit);
    m_layout.addWidget(&m_spacer);
    m_buttons.setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QObject::connect(&m_buttons, SIGNAL(rejected()), this, SLOT(reject()));
    QObject::connect(&m_buttons, SIGNAL(accepted()), this, SLOT(accept()));
    m_layout.addWidget(&m_buttons);
    setLayout(&m_layout);
}

void QInputOctalDialog::accept()
{
    quint16 value;
    if (! ParseOctalValue(m_edit.text().toLatin1().data(), &value))
    {
        QMessageBox::warning(this, 0, "Please enter correct octal value.");
        return;
    }

    *m_result = value;
    QDialog::accept();
}
