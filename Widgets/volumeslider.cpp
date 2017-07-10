#include "volumeslider.h"

#include <QMouseEvent>
#include <QStyleOptionSlider>
#include <QToolTip>
#include <QDebug>

VolumeSlider::VolumeSlider(QWidget *parent) :
    QSlider(parent),
    tooltip_timer(new QTimer)
{
    setStyleSheet("QSlider::groove:horizontal { "
                  "border: 1px solid #999999; "
                  "height: 1px; "
                  "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #B1B1B1, stop:1 #c4c4c4); "
                  "margin: 0px 0px; "
                  "} "
                  "QSlider::sub-page:horizontal {"
                  "background: qlineargradient(x1: 0, y1: 0,    x2: 0, y2: 1,"
                  "    stop: 0 #66e, stop: 1 #bbf);"
                  "background: qlineargradient(x1: 0, y1: 0.2, x2: 1, y2: 1,"
                  "    stop: 0 #bbf, stop: 1 #55f);"
                  "border: 1px solid #777;"
                  "height: 10px;"
                  "border-radius: 4px;"
                  "} "
                  "QSlider::handle:horizontal { "
                  "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #b4b4b4, stop:1 #8f8f8f); "
                  "border: 1px solid #5c5c5c; "
                  "width: 6px; "
                  "margin: -6px 0px; "
                  "} ");

    setRange(0, 100);
    setValue(25);
    setMaximumWidth(100);
    setOrientation(Qt::Horizontal);
    setMouseTracking(true);
    setCursor(Qt::PointingHandCursor);

    connect(this, SIGNAL(valueChanged(int)), SIGNAL(volumeChanged(int)));

    tooltip_timer->setInterval(50);
    connect(tooltip_timer, SIGNAL(timeout()), SLOT(showTooltip()));
}

VolumeSlider::~VolumeSlider()
{
    delete tooltip_timer;
}

void VolumeSlider::setVolume(int value)
{
    setValue(value);

    //QString text = "Volume: " + QString::number(value) + "%";
    QString text = QString("Volume: %1%").arg(value);
    setToolTip(text);
}

void VolumeSlider::showTooltip()
{
    QPoint point(mapToGlobal(QPoint(-3, -38)));
    QToolTip::showText(point, toolTip(), this);
}

void VolumeSlider::mousePressEvent(QMouseEvent *event)
{
    QStyleOptionSlider opt;
    initStyleOption(&opt);
    QRect sr = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);

    // If mouse is hovering over slider handle, this should be skipped
    // and drag-n-drop action performed instead ( default behavior )
    if (event->button() == Qt::LeftButton &&
        sr.contains(event->pos()) == false)
    {
        qint64 new_value = minimum() + ((maximum()-minimum()) * event->x()) / width();

        setVolume(new_value);
    }

    tooltip_timer->start();

    event->accept();
    QSlider::mousePressEvent(event);
}

void VolumeSlider::mouseReleaseEvent(QMouseEvent *event)
{
    tooltip_timer->stop();
    event->accept();
    QSlider::mouseReleaseEvent(event);
}
