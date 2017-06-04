#include "waitdialog.h"
#include "ui_waitdialog.h"

WaitDialog::WaitDialog(QDialog *parent) :
    QDialog(parent),
    ui(new Ui::WaitDialog)
{
    ui->setupUi(this);
    // hide close and "?" buttons
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint);

    connect(ui->button_ok, SIGNAL(clicked(bool)), this, SIGNAL(buttonOk_clicked()));
    connect(ui->button_cancel, SIGNAL(clicked(bool)), this, SIGNAL(buttonCancel_clicked()));
    connect(ui->button_open, SIGNAL(clicked(bool)), this, SIGNAL(buttonOpen_clicked()));
}

WaitDialog::~WaitDialog()
{
    delete ui;
}

void WaitDialog::enableButtonOpen(bool enabled)
{
    ui->button_open->setEnabled(enabled);
}

void WaitDialog::enableButtonCancel(bool enabled)
{
    ui->button_cancel->setEnabled(enabled);
}

void WaitDialog::enableButtonOk(bool enabled)
{
    ui->button_ok->setEnabled(enabled);
}

void WaitDialog::setStatusInfo(const QString &info)
{
    ui->label_status->setText(info);
}

void WaitDialog::showResult(const QStringList &errors)
{
    QString info = errors.empty() ? "Movie successfuly created." :
                                    "Failed to create a movie.\nDetails :\n" + errors.join("\n");

    ui->progressBar->hide();
    ui->label_statusHint->hide();
    ui->label_status->hide();
    ui->label_info->setText(info);

    ui->button_open->setEnabled(errors.empty());
    ui->button_ok->setEnabled(true);
    ui->button_cancel->setEnabled(false);
}
