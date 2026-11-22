#ifndef CLIENTDIALOG_H
#define CLIENTDIALOG_H

#include <QDialog>
#include "Client.h"

namespace Ui {
class ClientDialog;
}

class ClientDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ClientDialog(QWidget *parent = nullptr);
    ~ClientDialog();

    Client readClient() const;

signals:
    void submitRequested();

private:
    Ui::ClientDialog *ui;
};

#endif // CLIENTDIALOG_H
