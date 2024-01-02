#include "stdafx.h"
#include "qdialogs.h"
#include <QMessageBox>


//////////////////////////////////////////////////////////////////////


QInputOctalDialog::QInputOctalDialog(QWidget *parent, const QString & title, quint16 * value)
    : QDialog(parent, nullptr)
{
    m_result = value;

    setWindowTitle(title);
    resize(260, 120);
    m_labelOctal.setText(tr("Octal"));
    m_labelHex.setText(tr("Hex"));
    m_buttons.setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    m_layout.addWidget(&m_labelOctal, 0, 0);
    m_layout.addWidget(&m_labelHex, 0, 1);
    m_layout.addWidget(&m_editOctal, 1, 0);
    m_layout.addWidget(&m_editHex, 1, 1);
    m_layout.addWidget(&m_spacer, 2, 0);
    m_layout.addWidget(&m_buttons, 3, 0, 1, -1);
    setLayout(&m_layout);

    QObject::connect(&m_editOctal, SIGNAL(textEdited(QString)), this, SLOT(octalEdited(QString)));
    QObject::connect(&m_editHex, SIGNAL(textEdited(QString)), this, SLOT(hexEdited(QString)));
    QObject::connect(&m_buttons, SIGNAL(rejected()), this, SLOT(reject()));
    QObject::connect(&m_buttons, SIGNAL(accepted()), this, SLOT(accept()));

    char buffer[8];
    PrintOctalValue(buffer, *value);
    m_editOctal.setText(buffer);
    PrintHexValue(buffer, *value);
    m_editHex.setText(buffer);
    m_editOctal.selectAll();
}

void QInputOctalDialog::octalEdited(const QString &text)
{
    quint16 value;
    if (! ParseOctalValue(text.toLatin1().data(), &value))
    {
        m_editHex.setText(nullptr);
    }
    else
    {
        char buffer[8];
        PrintHexValue(buffer, value);
        m_editHex.setText(buffer);
    }
}

void QInputOctalDialog::hexEdited(const QString &text)
{
    quint16 value;
    if (! ParseHexValue(text.toLatin1().data(), &value))
    {
        m_editOctal.setText(nullptr);
    }
    else
    {
        char buffer[8];
        PrintOctalValue(buffer, value);
        m_editOctal.setText(buffer);
    }
}

void QInputOctalDialog::accept()
{
    QString text = m_editOctal.text();
    quint16 value;
    if (text.isEmpty() || !ParseOctalValue(text.toLatin1().data(), &value))
    {
        QMessageBox::warning(this, nullptr, tr("Please enter correct octal value."));
        return;
    }

    *m_result = value;
    QDialog::accept();
}


//////////////////////////////////////////////////////////////////////


QAboutDialog::QAboutDialog(QWidget * parent)
    : QDialog(parent, nullptr)
{
    setWindowTitle(tr("About"));

    m_topLabel.setText(tr("BKBTL Qt Version 1.0\nCopyright (C) 2009-2024"));
    m_authorsLabel.setText(tr("Author:\r\nNikita Zimin (nzeemin@gmail.com)"));
    m_thanksLabel.setText(tr("Special thanks to:\nAlexey Kisly"));
    m_linkLabel.setText(tr("<a href=\"https://github.com/nzeemin/bkbtl-qt\">https://github.com/nzeemin/bkbtl-qt</a>"));
    m_disclaimerLabel.setText(tr("This program is provided by authors and contributors AS IS, with absolutely no warranty of any kind.  Please use it on your own risk."));
    m_bottomLabel.setText(tr("Build date:\t%1 %2\nQt version:\t%3").arg(__DATE__).arg(__TIME__).arg(QT_VERSION_STR));

    m_linkLabel.setOpenExternalLinks(true);
    m_disclaimerLabel.setWordWrap(true);

    setMinimumSize(420, 320);

    m_layout.addWidget(&m_topLabel);
    m_layout.addWidget(&m_authorsLabel);
    m_layout.addWidget(&m_thanksLabel);
    m_layout.addWidget(&m_linkLabel);
    m_layout.addWidget(&m_disclaimerLabel);
    m_layout.addWidget(&m_bottomLabel);
    m_buttons.setStandardButtons(QDialogButtonBox::Ok);
    QObject::connect(&m_buttons, SIGNAL(accepted()), this, SLOT(accept()));
    m_layout.addWidget(&m_buttons);
    setLayout(&m_layout);
}


//////////////////////////////////////////////////////////////////////
