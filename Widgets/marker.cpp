#include "marker.h"
#include "ui_marker.h"
#include <QTime>

Marker::Marker(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Marker)
{
    ui->setupUi(this);

    ui->button_setMarker->setToolTip("Create marker at current time");
    ui->button_clear->setToolTip("Clear this marker");
    ui->button_move->setToolTip("Move to this marker's time");

    connect(ui->button_clear, SIGNAL(clicked(bool)), this, SLOT(clear()));
    connect(ui->button_move, SIGNAL(clicked(bool)), this, SIGNAL(clicked_move()));
    connect(ui->button_setMarker, SIGNAL(clicked(bool)), this, SIGNAL(clicked_setMarker()));

    setTime(QTime(0, 0, 0));
    setDefault(QTime(0, 0, 0));
    clear();
}

Marker::Marker(const QString &description, QWidget *parent) :
    Marker(parent)
{
    setDescription(description);
}

Marker::~Marker()
{
    delete ui;
}

void Marker::setDescription(const QString &text)
{
    ui->label_description->setText(text);
}

QString Marker::getDescription() const
{
    return ui->label_description->text();
}

void Marker::setTime(const QTime &time)
{
    this->time = time;
    //QString format = time.hour() > 0 ? "HH:mm:ss.zzz" : "mm:ss.zzz";
    ui->label_time->setText(time.toString("HH:mm:ss.zzz"));
    activate(true);
}

QTime Marker::getTime() const
{
    return time;
}

void Marker::setDefault(const QTime &time)
{
    default_time = time;
}

QTime Marker::getDefault() const
{
    return default_time;
}

void Marker::activate(bool enabled)
{
    ui->button_clear->setEnabled(enabled);
    ui->button_setMarker->setEnabled(!enabled);
}

void Marker::clear()
{
    setTime(default_time);
    activate(false);
}
