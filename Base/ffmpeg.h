#ifndef FFMPEG_H
#define FFMPEG_H

#include <QObject>
#include <QTime>
#include <QProcess>
#include <QCoreApplication>
#include <QSize>
#include <QSharedPointer>


class FFmpeg : public QObject
{
    Q_OBJECT

public:
    enum Transform {
        None,
        Reverse,
        Mirror,
        Loop,
        InsertKeyframes
    };
    Q_ENUM(Transform)

    enum GifQuality {
        High,
        Low
    };
    Q_ENUM(GifQuality)

    enum SeekBehavior {
        Fast,
        Slow,
        CombinedFast,
        CombinedSlow
    };
    Q_ENUM(SeekBehavior)

    static const char *time_format;
    static const int sizeOriginal = -2;
    static const int sizeScale = -1;
    static const int fpsOriginal = -1;

public:
    explicit FFmpeg(QObject *parent = 0);
    ~FFmpeg();

    QStringList getErrors() const;

    void setTime(const QTime &start, const QTime &end);
    void setFiles(const QString &input, const QString &output);
    void setTimeout(int msecs);
    void setFps(int fps);
    void setFrameSize(int width, int height);
    void setFrameSize(const QSize &size);
    void setBehavior(Transform behavior);

public slots:
    void start();
    void terminate();
    void clearTmp();

signals:
    void stateChanged(const QString &state);
    void finished();
    void finished(const QStringList& errors);

protected slots:
    void handleError(QProcess::ProcessError error);
    void handleError(const QString &error);
    void currentProcessChanged(const QString &proc_name);

protected:
    static QStringList seek(const QTime &start);
    static QStringList seek(const QTime &start, SeekBehavior behavior, int time_buffer = -1);
    static QStringList seek(qint64 msecs, SeekBehavior behavior, int time_buffer = -1);

protected:
    void insertKeyframes(const QString &file_in, const QString &file_out, const QString &start, const QString &frequency);
    void insertKeyframes(const QString &file_in, const QString &file_out, const QTime &start, const QTime &end);
    void reverse(const QString &file_in, const QString &file_out);
    void concat(const QString &file_in_1, const QString &file_in_2, const QString &file_out);
    void copy(const QString &file_in, const QString &file_out, const QTime &start, const QTime &duration);
    void copy(const QString &file_in, const QString &file_out);
    void mirror(const QString &file_in, const QString &file_out);
    void transform(const QString &file_in, const QString &file_out);

    void makeGif(const QString &file_in, const QString &file_out, const QTime &start, const QTime &duration);
    void makeVideo(const QString &file_in, const QString &file_out, const QTime &start, const QTime &duration);

    void makePalette(const QString &file_in, const QTime &start, const QTime &duration);
    void createFilter();
    void run();

protected:
    QString input_file, output_file;  
    QTime time_start, time_end;

    QStringList filter_base;
    int fps;
    int timeout;
    QSize frame_size;
    Transform behavior;
    GifQuality gif_quality;

    QScopedPointer<QProcess> proc;
    QStringList args;

    QStringList err_list;
    QString current_process;
};

#endif // FFMPEG_H
