#ifndef WAITDIALOG_H
#define WAITDIALOG_H

#include <QDialog>

namespace Ui {
class WaitDialog;
}

class WaitDialog : public QDialog
{
    Q_OBJECT

public:
    enum ReturnAction {
        Ok,
        Open,
        Show
    };
    Q_ENUM(ReturnAction)

public:
    explicit WaitDialog(QDialog *parent = 0);
    ~WaitDialog();

public slots:
    void showResult(const QStringList &errors);
    void setStatusInfo(const QString &info);

signals:
    void buttonCancel_clicked();

private:
    Ui::WaitDialog *ui;
};

#endif // WAITDIALOG_H
