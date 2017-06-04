#include "ffmpeg.h"

#include <QDebug>
#include <QDir>

// Internal variables and functions

Q_GLOBAL_STATIC_WITH_ARGS(QString, ffmpeg, (QCoreApplication::applicationDirPath() + "/ffmpeg.exe"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, tmpdir, (QCoreApplication::applicationDirPath() + "/tmp"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, palette, (QCoreApplication::applicationDirPath() + "/tmp/palette.png"))

static const char time_format[] = "HH:mm:ss.zzz";
static const int time_buffer = 10; // for fast seek

static QString getExtension(const QString &filename)
{
    return filename.split(".").last();
}

// Seek by keyframes. Should be used in combination with slow seek
static QStringList seekFast(const QTime &start, bool forced = false)
{
    if (QTime(0 ,0 ,0).secsTo(start) < time_buffer && !forced)
        return QStringList();

    QStringList list;
    QString str_fast;

    if (forced)
        str_fast = start.toString(time_format);
    else
        str_fast = start.addSecs(-time_buffer).toString(time_format);

    list << "-ss" << str_fast;

    return list;
}

// If forced = false, performs slow seek combined with fast one
// Otherwise complete slow seek will be done
static QStringList seekSlow(const QTime &start, bool forced = false)
{
    if (QTime(0, 0, 0).msecsTo(start) == 0)
        return QStringList();

    QStringList list;

    if (QTime(0 ,0 ,0).secsTo(start) < time_buffer || forced)
    {
        list << "-ss" << start.toString(time_format);
    }
    else if (!forced)
    {
        list << "-ss" << QTime(0, 0, time_buffer).toString(time_format);
    }

    return list;
}

// Internal end


FFmpeg::FFmpeg(QObject *parent) :
    QObject(parent),
    proc(nullptr)
{
    frame_size = QSize(sizeScale, sizeScale);
    fps = fpsOriginal;
    behavior = Normal;
    gif_quality = High;

    timeout = 1000 * 60 * 10;
}

FFmpeg::~FFmpeg()
{
    // In case start() was somehow interrupted and process not finished properly
    if (proc && proc->pid())
        terminate();
}

QStringList FFmpeg::getErrors() const
{
    return err_list;
}

void FFmpeg::setTime(const QTime &start, const QTime &end)
{
    time_start = start;
    time_end = end;
}

void FFmpeg::setFiles(const QString &input, const QString &output)
{
    input_file = input;
    output_file = output;
}

void FFmpeg::setTimeout(int msecs)
{
    timeout = msecs;
}

void FFmpeg::setFps(int fps)
{
    this->fps = fps;
}

void FFmpeg::setFrameSize(int width, int height)
{
   frame_size.setHeight(height);
   frame_size.setWidth(width);
}

void FFmpeg::setFrameSize(const QSize &size)
{
    setFrameSize(size.width(), size.height());
}

void FFmpeg::setBehavior(FFmpeg::Behavior behavior)
{
    this->behavior = behavior;
}

void FFmpeg::transform(const QString &file_in, const QString &file_out)
{
    switch (behavior) {
    case Behavior::Loop:
    {
        QString tmp_file_rev = *tmpdir + "/tmp_copy_rev." + getExtension(file_in);

        reverse(file_in, tmp_file_rev);
        concat(file_in, tmp_file_rev, file_out);

        break;
    }
    case Behavior::Keyframes:
    {
        /*if (getExtension(file_out) == "gif")
            handleError("Keyframes cannot apply to GIF format.");
        else
            insertKeyframes(file_in, file_out, "0", "0.1");*/
        insertKeyframes(file_in, file_out, "0", "0.1");

        break;
    }
    case Behavior::Reverse:
    {
        reverse(file_in, file_out);
        break;
    }

    default:
    {
        qDebug() << "Normal behavior causes no transform";
        break;
    }
    }
}

void FFmpeg::makePalette(const QString &file_in, const QTime &start, const QTime &duration)
{
    emit stateChanged("Generating palette");

    if (duration.isNull())
        handleError("Duration is Null.");

    QString str_duration = duration.toString(time_format);
    QStringList filter = filter_base; filter << "palettegen";

    args.clear();
    args << seekFast(start, true)
         << "-t" << str_duration
         << "-i" << file_in
         << "-vf" << filter.join(",")
         << *palette;

    run();
}

void FFmpeg::makeGif(const QString &file_in, const QString &file_out, const QTime &start, const QTime &duration)
{
    makePalette(file_in, start, duration);

    emit stateChanged("Creating GIF");

    if (duration.isNull())
        handleError("Duration is Null.");

    QString str_duration = duration.toString(time_format);
    QStringList filter = filter_base; filter << "paletteuse";

    args.clear();
    args << seekFast(start, true)
         << "-i" << file_in
         << "-i" << *palette
         << "-t" << str_duration
         << "-lavfi" << filter.join(",")
         << file_out;

    run();

    if (behavior != Normal)
        transform(file_out, output_file);
}

void FFmpeg::makeVideo(const QString &file_in, const QString &file_out, const QTime &start, const QTime &duration)
{
    emit stateChanged("Creating video");

    if (duration.isNull())
        handleError("Duration is Null.");

    QString str_duration = duration.toString(time_format);
    QStringList filter = filter_base;

    args.clear();
    args << seekFast(start)
         << "-i" << file_in
         << seekSlow(start)
         << "-t" << str_duration
         << "-lavfi" << filter.join(",")
         << file_out;

    run();

    if (behavior != Normal)
        transform(file_out, output_file);
}

void FFmpeg::copy(const QString &file_in, const QString &file_out, const QTime &start, const QTime &duration)
{
    emit stateChanged("Copying");

    if (start.isNull())
        qDebug() << "Warning! FFmpeg::copy: Start time is Null.";
    if (duration.isNull())
        handleError("Duration is Null.");

    QString str_duration = duration.toString(time_format);

    args.clear();
    args << seekFast(start)
         << "-i" << file_in
         << seekSlow(start)
         << "-t" << str_duration
         << "-c" << "copy"
         << file_out;

    run();
}

void FFmpeg::copy(const QString &file_in, const QString &file_out)
{
    emit stateChanged("Copying");

    args.clear();
    args << "-i" << file_in
         << file_out;

    run();
}

void FFmpeg::insertKeyframes(const QString &file_in, const QString &file_out, const QString &start, const QString &frequency)
{
    emit stateChanged("Inserting key frames");

    QStringList expr;
    expr << "expr:if(isnan(prev_forced_t)";
    expr << "gte(t," + start + ")";
    expr << "gte(t,prev_forced_t+" + frequency + "))";

    args.clear();
    args << "-i" << file_in
         << "-force_key_frames" << expr.join(",")
         << file_out;

    run();
}

void FFmpeg::insertKeyframes(const QString &file_in, const QString &file_out, const QTime &start, const QTime &end)
{
    emit stateChanged("Inserting key frames");

    if (start.isNull())
        handleError("Start time is Null");
    if (end.isNull())
        handleError("End time is Null");

    QString str_start = start.toString(time_format);
    QString str_end = end.toString(time_format);

    args.clear();
    args << "-i" << file_in
         << "-force_key_frames" << QString(str_start + "," + str_end)
         << file_out;

    run();
}

void FFmpeg::reverse(const QString &file_in, const QString &file_out)
{
    emit stateChanged("Reversing");

    args.clear();
    args  << "-i" << file_in
          << "-vf" << "reverse"
          << "-af" << "areverse"
          << file_out;

    run();
}

void FFmpeg::concat(const QString &file_in_1, const QString &file_in_2, const QString &file_out)
{
   emit stateChanged("Concatenating");

    args.clear();
    /*args << "-i" << file_in_1
         << "-i" << file_in_2
         << "-filter_complex" << "[0:v:0] [0:a:0] [1:v:0] [1:a:0] concat=n=2:v=1:a=1 [v] [a]"
         << "-map" << "[v]"
         << "-map" << "[a]"
         << file_out;*/

    // Video only

    args << "-i" << file_in_1
         << "-i" << file_in_2
         << "-filter_complex" << "[0:v:0] [1:v:0] concat=n=2:v=1:a=0 [v]"
         << "-map" << "[v]"
         << file_out;

    run();
}

void FFmpeg::run()
{
    qDebug() << args;

    if (err_list.empty())
    {
        proc->setProgram(*ffmpeg);
        proc->setArguments(args);
        proc->start();
        proc->waitForFinished(timeout);
    }
    else
    {
        handleError("Failed to start due to errors earlier");
    }
}

void FFmpeg::handleError(QProcess::ProcessError error)
{
    QString err_msg;
    switch(error) {
    case QProcess::FailedToStart:
        err_msg = "Failed to start";
        break;
    case QProcess::Crashed:
        err_msg = "Crashed";
        break;
    case QProcess::Timedout:
        err_msg = "Timedout";
        break;
    case QProcess::WriteError:
        err_msg = "Write Error";
        break;
    case QProcess::ReadError:
        err_msg = "Read Error";
        break;
    case QProcess::UnknownError:
        err_msg = "Unknown Error";
        break;
    }

    handleError(err_msg);
}

void FFmpeg::handleError(const QString &error)
{
    err_list << current_process + " : " + error;

    qDebug() << err_list;
}

void FFmpeg::currentProcessChanged(const QString &proc_name)
{
    current_process = proc_name;
    qDebug() << "Current process : " << current_process << endl;
}

void FFmpeg::start()
{
    proc = QSharedPointer<QProcess>(new QProcess);
    connect(this, SIGNAL(stateChanged(QString)), this, SLOT(currentProcessChanged(QString)));
    connect(proc.data(), SIGNAL(errorOccurred(QProcess::ProcessError)), this, SLOT(handleError(QProcess::ProcessError)));

    clearTmp();
    err_list.clear();
    createFilter();

    stateChanged("Preparing");

    if (!QDir(QCoreApplication::applicationDirPath()).mkdir("tmp"))
        handleError("Failed to create temporary directory");
    if (!QFile(*ffmpeg).exists())
        handleError("ffmpeg not found");

    QTime duration = QTime(0, 0, 0).addMSecs(time_start.msecsTo(time_end));
    qDebug() << duration;
    // tmp file is used for further transformations
    QString file_out = behavior == Normal ?
                                   output_file :
                                   *tmpdir + "/tmp_copy." + getExtension(output_file);

    if (getExtension(output_file) == "gif")
    {
        // Simple copying for low-quality gif
        if (gif_quality == High)
            makeGif(input_file, file_out, time_start, duration);
        else
            copy(input_file, file_out, time_start, duration);
    }
    else
    {
        makeVideo(input_file, file_out, time_start, duration);
    }

    emit stateChanged("Finished");
    emit finished();
    emit finished(err_list);
}

void FFmpeg::terminate()
{
    // Stops current process from running and prevents it from starting again
    // Calling this function in the same thread as 'start()' will do nothing
    if (proc->pid())
    {
        proc->kill();
        handleError("Terminated");
    }
}

void FFmpeg::clearTmp()
{
    emit stateChanged("Clearing temporary files");

    if (QDir(*tmpdir).exists())
        if (!QDir(*tmpdir).removeRecursively())
            handleError("Failed to remove temporary directory");
}

void FFmpeg::createFilter()
{
    // Creating ffmpeg filter for fps and frame size
    if (fps != fpsOriginal)
        filter_base << "fps=" + QString::number(fps);

    QString str_width, str_height;

    if (frame_size.width() == sizeOriginal)
        str_width = "-iw";
    else if (frame_size.width() == sizeScale)
        str_width = "-1";
    else
        str_width = QString::number(frame_size.width());

    if (frame_size.height() == sizeOriginal)
        str_height = "-ih";
    else if (frame_size.height() == sizeScale)
        str_height = "-1";
    else
        str_height = QString::number(frame_size.height());


    QString str_size = "scale=" + str_width + ":" + str_height;

    filter_base << str_size;
}
