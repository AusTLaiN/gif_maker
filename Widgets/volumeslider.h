#ifndef VOLUMESLIDER_H
#define VOLUMESLIDER_H

#include <QSlider>
#include <QTimer>

class VolumeSlider : public QSlider
{
    Q_OBJECT

public:
    explicit VolumeSlider(QWidget *parent = 0);
    ~VolumeSlider();

signals:
    void volumeChanged(int value);

public slots:
    void setVolume(int value);

protected slots:
    void showTooltip();

protected:
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;

protected:
    QTimer *tooltip_timer;
};

#endif // VOLUMESLIDER_H
