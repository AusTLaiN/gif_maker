#include "durationslider.h"

#include <QMouseEvent>
#include <QStyle>
#include <QStyleOptionSlider>
#include <QToolTip>
#include <QDebug>
#include <QTime>

DurationSlider::DurationSlider(QWidget *parent) :
    QSlider(parent)
{
    setStyleSheet("QSlider::groove:horizontal { "
                  "border: 1px solid #999999; "
                  "height: 3px; "
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
                  "margin: -5px 0px; "
                  "} ");

    setRange(0, 0);
    setOrientation(Qt::Horizontal);
    setMouseTracking(true);

    connect(this, SIGNAL(sliderMoved(int)), this, SLOT(onSliderMoved(int)));
}

void DurationSlider::setPosition(qint64 msecs)
{
    if (value() != msecs / step)
        setValue(msecs / step);
}

void DurationSlider::setDuration(qint64 msecs)
{
    setRange(0, msecs / step);
}

void DurationSlider::onSliderMoved(int pos)
{
    // Emitting signal with current position in milliseconds
    emit positionChanged(pos * step);
}

void DurationSlider::mousePressEvent(QMouseEvent *event)
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

        setValue(new_value);
        onSliderMoved(new_value);
    }

    event->accept();
    QSlider::mousePressEvent(event);
}

void DurationSlider::mouseMoveEvent(QMouseEvent *event)
{
    QPoint location;
    location.setX(event->globalX());
    location.setY(this->mapToGlobal(QPoint(-5, -45)).y());

    qint64 val = minimum() + ((maximum()-minimum()) * event->x()) / width();

    QTime t = QTime(0, 0, 0).addMSecs(val * step);
    QStringList time_str;

    time_str << t.toString(t.hour() ? "HH:mm:ss" : "mm:ss");
    time_str << QString::number(t.msec() / 100);

    // if tooltip drawing started, but not finished yet
    // prevents artifacts and blinking while moving mouse fast
    if (!(QToolTip::isVisible() == false && QToolTip::text().isEmpty() == false))
        QToolTip::showText(location, time_str.join("."), this, QRect(0, 0, 1, 1));

    event->accept();
    QSlider::mouseMoveEvent(event);
}
