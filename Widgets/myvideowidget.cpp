#include "myvideowidget.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QDebug>

MyVideoWidget::MyVideoWidget(QWidget *parent) :
    QVideoWidget(parent),
    timer_hide_mouse(nullptr)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QPalette p = palette();
    p.setColor(QPalette::Window, Qt::black);
    setPalette(p);

    setAttribute(Qt::WA_OpaquePaintEvent);
    setMouseTracking(true);

    connect(&timer_hide_mouse, SIGNAL(timeout()), this, SLOT(hideMouse()));
    connect(this, &fullScreenChanged, [this](bool fullScreen){
        if (fullScreen)
        {
            beginHiding();
        }
        else
        {
            setCursor(Qt::ArrowCursor);
        }
    });
}

void MyVideoWidget::hideMouse()
{
    timer_hide_mouse.stop();
    setCursor(Qt::BlankCursor);
}

void MyVideoWidget::beginHiding()
{
    timer_hide_mouse.stop();
    timer_hide_mouse.setInterval(2000);
    timer_hide_mouse.start();

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

    if (isFullScreen())
    {
        beginHiding();
    }

    event->accept();
}
