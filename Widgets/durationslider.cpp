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
    changed_inside = false;

    setStyleSheet("QSlider::groove:horizontal { "
                  "border: 1px solid #999999; "
                  "height: 4px; "
                  "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #B1B1B1, stop:1 #c4c4c4); "
                  "margin: 0px 0; "
                  "} "
                  "QSlider::handle:horizontal { "
                  "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #b4b4b4, stop:1 #8f8f8f); "
                  "border: 1px solid #5c5c5c; "
                  "width: 8px; "
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
    {
        setValue(msecs / step);
        changed_inside = false;
    }
}

void DurationSlider::setDuration(qint64 msecs)
{
    setRange(0, msecs / step);
}

void DurationSlider::onSliderMoved(int pos)
{
    // Emitting signal with current position in milliseconds
    changed_inside = true;
    emit positionChanged(pos * step);
}

void DurationSlider::mousePressEvent(QMouseEvent *event)
{
    // If mouse is hovering over slider handle, this should be skipped
    // and drag-n-drop action performed instead ( default behavior )
    QStyleOptionSlider opt;
    initStyleOption(&opt);
    QRect sr = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);

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
    /*static QPoint prev_pos;

    if (qAbs(event->globalPos().x() - prev_pos.x()) < 7
        && QToolTip::text().isEmpty() == false) {
        //prev_pos = event->globalPos();
        return;
    }

    prev_pos = event->globalPos();*/

    QPoint location;
    location.setX(event->globalX());
    location.setY(this->mapToGlobal(QPoint(-5, -50)).y());

    qint64 val = minimum() + ((maximum()-minimum()) * event->x()) / width();

    //qDebug() << "Draw" << QTime::currentTime();

    QTime t = QTime(0, 0, 0).addMSecs(val * step);
    QStringList time_str;

    time_str << t.toString(t.hour() ? "HH:mm:ss" : "mm:ss");
    time_str << QString::number(t.msec() / 100);

    if (QToolTip::isVisible() == false && QToolTip::text().isEmpty() == false)
        0;
    else
        QToolTip::showText(location, time_str.join("."), this, QRect(0, 0, 1, 1));

    event->accept();
    QSlider::mouseMoveEvent(event);
}
