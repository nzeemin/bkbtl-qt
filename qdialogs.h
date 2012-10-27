#ifndef QDIALOGS_H
#define QDIALOGS_H

#include <QWidget>
#include <QDialog>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QLabel>


class QInputOctalDialog : public QDialog
{
    Q_OBJECT

public:
    QInputOctalDialog(QWidget * parent, const QString & title, const QString & prompt, quint16 * value);

public slots:
    virtual void accept();

private:
    quint16 * m_result;
    QVBoxLayout m_layout;
    QLabel m_label;
    QLineEdit m_edit;
    QWidget m_spacer;
    QDialogButtonBox m_buttons;
};

#endif // QDIALOGS_H
