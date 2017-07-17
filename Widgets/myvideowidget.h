#ifndef MYVIDEOWIDGET_H
#define MYVIDEOWIDGET_H

#include <QVideoWidget>
#include <QElapsedTimer>
#include <QTimer>

class MyVideoWidget : public QVideoWidget
{
    Q_OBJECT

public:
    explicit MyVideoWidget(QWidget *parent = 0);

signals:
    void mousePressed();

public slots:
    void setFullScreenWidget(QWidget *widget);

protected slots:
    void hideMouse();
    void beginHiding();
    void stopHiding();

    void onFullScreenChanged(bool fullscreen);

protected:
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;

protected:
    QElapsedTimer timer;
    QTimer timer_hide_mouse;
    QWidget *fullscreen_widget;
};

#endif // MYVIDEOWIDGET_H
