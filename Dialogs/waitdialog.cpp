#include "waitdialog.h"
#include "ui_waitdialog.h"

WaitDialog::WaitDialog(QDialog *parent) :
    QDialog(parent),
    ui(new Ui::WaitDialog)
{
    ui->setupUi(this);
    // hide close and "?" buttons
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint);

    connect(ui->button_cancel, SIGNAL(clicked(bool)), this, SIGNAL(buttonCancel_clicked()));  
    connect(ui->button_ok, &QPushButton::clicked, [this](){
        done(Ok);
    });
    connect(ui->button_open, &QPushButton::clicked, [this](){
        done(Open);
    });
    connect(ui->button_show, &QPushButton::clicked, [this](){
        done(Show);
    });

    ui->button_ok->setVisible(false);
    ui->button_show->setVisible(false);
    ui->button_open->setVisible(false);

    ui->button_ok->setCursor(Qt::PointingHandCursor);
    ui->button_open->setCursor(Qt::PointingHandCursor);
    ui->button_show->setCursor(Qt::PointingHandCursor);
    ui->button_cancel->setCursor(Qt::PointingHandCursor);
}

WaitDialog::~WaitDialog()
{
    delete ui;
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

    ui->button_cancel->setVisible(false);
    ui->button_ok->setVisible(true);
    ui->button_show->setVisible(true);
    ui->button_open->setVisible(true);

    ui->button_open->setEnabled(errors.empty());
    ui->button_show->setEnabled(errors.empty());
    ui->button_ok->setEnabled(true);
    //ui->button_cancel->setEnabled(false);

    ui->button_ok->setDefault(true);
}
