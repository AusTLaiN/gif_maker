#include "myvideowidget.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QDebug>
#include <QLabel>
#include <QSlider>
#include <QPushButton>

MyVideoWidget::MyVideoWidget(QWidget *parent) :
    QVideoWidget(parent),
    timer_hide_mouse(nullptr),
    fullscreen_widget(nullptr)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QPalette p = palette();
    p.setColor(QPalette::Window, Qt::black);
    setPalette(p);

    setAttribute(Qt::WA_OpaquePaintEvent);
    setMouseTracking(true);

    connect(&timer_hide_mouse, SIGNAL(timeout()), this, SLOT(hideMouse()));
    connect(this, SIGNAL(fullScreenChanged(bool)), SLOT(onFullScreenChanged(bool)));
}

void MyVideoWidget::setFullScreenWidget(QWidget *widget)
{
    fullscreen_widget = widget;
    if (fullscreen_widget)
    {
        //fullscreen_widget->setParent(this);
        fullscreen_widget->setMouseTracking(true);
        fullscreen_widget->hide();
        fullscreen_widget->raise();
    }
}

void MyVideoWidget::hideMouse()
{
    timer_hide_mouse.stop();
    setCursor(Qt::BlankCursor);
}

void MyVideoWidget::beginHiding()
{
    timer_hide_mouse.stop();
    timer_hide_mouse.setInterval(1500);
    timer_hide_mouse.start();

}

void MyVideoWidget::stopHiding()
{
    timer_hide_mouse.stop();
    setCursor(Qt::ArrowCursor);
}

void MyVideoWidget::onFullScreenChanged(bool fullscreen)
{
    if (fullscreen)
    {
        beginHiding();

        if (fullscreen_widget)
        {
            fullscreen_widget->setGeometry(0, 0, this->width(), fullscreen_widget->height());

            QPoint pos = this->pos();
            pos += QPoint(0, this->height());
            pos -= QPoint(0, fullscreen_widget->height());

            /*qDebug() << "VideoWidget height = " << this->height();
            qDebug() << "FullScreenWidget height = " << fullscreen_widget->height();
            qDebug() << "Pos = " << pos;*/

            fullscreen_widget->move(pos);
        }
    }
    else
    {
        setCursor(Qt::ArrowCursor);

        if (fullscreen_widget)
        {
            fullscreen_widget->hide();
        }
    }
}

void MyVideoWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape && isFullScreen())
    {
        setFullScreen(false);
        event->accept();
    }
    else if (event->key() == Qt::Key_Enter && event->modifiers() & Qt::Key_Alt)
    {
        setFullScreen(!isFullScreen());
        event->accept();
    }
    else
    {
        QVideoWidget::keyPressEvent(event);
    }
}

void MyVideoWidget::mouseDoubleClickEvent(QMouseEvent *event)
{   
    if (timer.elapsed() > 100)
    {
        setFullScreen(!isFullScreen());
        event->accept();
        emit mousePressed();
    }
}

void MyVideoWidget::mousePressEvent(QMouseEvent *event)
{
    emit mousePressed();
    event->accept();
    timer.start();
}

void MyVideoWidget::mouseMoveEvent(QMouseEvent *event)
{
    setCursor(Qt::ArrowCursor);

    if (fullscreen_widget)
    {
        QRect rect = fullscreen_widget->geometry();
        //qDebug() << "Event pos = " << event->pos();

        if (isFullScreen())
        {
            if (rect.contains(event->pos()))
            {
                fullscreen_widget->show();
                stopHiding();
            }
            else
            {
                fullscreen_widget->hide();
                beginHiding();
            }
        }
    }
    else
    {
        if (isFullScreen())
            beginHiding();
    }


    event->accept();
}
