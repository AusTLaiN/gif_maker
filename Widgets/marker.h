#ifndef MARKER_H
#define MARKER_H

#include <QWidget>
#include <QTime>

namespace Ui {
class Marker;
}

class Marker : public QWidget
{
    Q_OBJECT

public:
    explicit Marker(QWidget *parent = 0);
    Marker(const QString &description, QWidget *parent = 0);
    ~Marker();

    QString getDescription() const;
    QTime getTime() const;
    QTime getDefault() const;

public slots:
    void setDescription(const QString &text);
    void setTime(const QTime &time);
    void activate(bool enabled = true);
    void setDefault(const QTime &time);
    void clear();

signals:
    void clicked_setMarker();
    void clicked_move();

protected:
    QTime time;
    QTime default_time;

private:
    Ui::Marker *ui;
};

#endif // MARKER_H
