#ifndef DURATIONSLIDER_H
#define DURATIONSLIDER_H

#include <QSlider>


class DurationSlider : public QSlider
{
    Q_OBJECT

public:
    static const int step = 100; // slider step size in milliseconds

public:
    explicit DurationSlider(QWidget *parent = nullptr);
    ~DurationSlider() = default;

public slots:
    // These slots should be used instead of base setValue() and setRange()
    void setPosition(qint64 msecs);
    void setDuration(qint64 msecs);

signals:
    void positionChanged(qint64 msecs);

protected slots:
    void onSliderMoved(int pos);

protected:
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
};

#endif // DURATIONSLIDER_H
