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

    // Shows whether the slider changed it's value from inside ( moved by user )
    // or from outside ( by public slot )
    bool modifiedInternally() const { return changed_inside; }
    void enableModification() { changed_inside = false; }

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
protected:
    bool changed_inside;

};

#endif // DURATIONSLIDER_H
