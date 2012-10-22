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
    QInputOctalDialog(QWidget * parent, const QString & title, const QString & prompt, uint16_t * value);

public slots:
    virtual void accept();

private:
    uint16_t * m_result;
    QVBoxLayout m_layout;
    QLabel m_label;
    QLineEdit m_edit;
    QWidget m_spacer;
    QDialogButtonBox m_buttons;
};

#endif // QDIALOGS_H
