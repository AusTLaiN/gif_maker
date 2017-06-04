#include "makemoviedialog.h"
#include "ui_makemoviedialog.h"
#include "waitdialog.h"

#include <QMessageBox>
#include <QThread>
#include <QFile>
#include <QDir>
#include <QDebug>

MakeMovieDialog::MakeMovieDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MakeMovieDialog)
{
    ui->setupUi(this);
    // Hide "?" button
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    setWindowTitle("Movie options");

    file_options = new FileOptions;

    ui->verticalLayout_5->addWidget(file_options);
    ui->verticalLayout_5->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));

    ui->comboBox_height->setToolTip("Output movie height.\n"
                                    "Scale works only if another dimension is set.\n"
                                    "Scaling both dimensions will give the original size");
    ui->comboBox_width->setToolTip("Output movie width.\n"
                                    "Scale works only if another dimension is set.\n"
                                    "Scaling both dimensions will give the original size");
    ui->comboBox_fps->setToolTip("Output movie fps");


    connect(ui->button_create, SIGNAL(clicked(bool)), this, SLOT(createMovie()));
    connect(ui->comboBox_transform, SIGNAL(currentTextChanged(QString)), this, SLOT(comboBoxTransform_textChanged(QString)));

    resize(640, 150);
    ui->comboBox_fps->setCurrentText("20");
    comboBoxTransform_updateTooltip();
}

MakeMovieDialog::MakeMovieDialog(const QString &input_file_fullname, QWidget *parent) :
    MakeMovieDialog(parent)
{
    input_filename = input_file_fullname;

    QString filename = input_filename.split("/").last();
    int i = filename.length();
    while(filename[--i] != '.');

    QString name_only = filename.mid(0, i);
    QString extension = filename.mid(i, filename.length() - i);

    file_options->setFileName(name_only);
    file_options->setExtension(extension);
}

MakeMovieDialog::~MakeMovieDialog()
{
    delete ui;
}

FFmpeg::Behavior MakeMovieDialog::getTransformation() const
{
    return static_cast<FFmpeg::Behavior>(ui->comboBox_transform->currentIndex());
}

QSize MakeMovieDialog::getSize() const
{
    QSize size;
    QString str_width = ui->comboBox_width->currentText();
    QString str_height = ui->comboBox_height->currentText();

    if (str_width == "Original")
        size.setWidth(FFmpeg::sizeOriginal);
    else if (str_width == "Scale")
        size.setWidth(FFmpeg::sizeScale);
    else
        size.setWidth(str_width.toInt());

    if (str_height == "Original")
        size.setHeight(FFmpeg::sizeOriginal);
    else if (str_height == "Scale")
        size.setHeight(FFmpeg::sizeScale);
    else
        size.setHeight(str_height.toInt());

    return size;
}

int MakeMovieDialog::getFps() const
{
    QString str_fps = ui->comboBox_fps->currentText();

    if (str_fps == "Original")
        return FFmpeg::fpsOriginal;
    else
        return str_fps.toInt();
}

QString MakeMovieDialog::getResult() const
{
    return result_file;
}

void MakeMovieDialog::setTime(const QTime &start, const QTime &end, const QTime &total)
{
    t_start = start;
    t_end = end;
    t_total = total;

    allowTransformation(start.msecsTo(end) == QTime(0, 0, 0).msecsTo(total));
}

void MakeMovieDialog::allowTransformation(bool allowed)
{
    ui->comboBox_transform->setEnabled(allowed);

    if (!allowed)
    {
        ui->comboBox_transform->setToolTip("Transformation can be applied only to the entire movie.\n"
                                           "Clear both markers to enable this option.");
    }
}

void MakeMovieDialog::createMovie()
{
    if (!QDir(file_options->getDirectory()).exists())
    {
        QMessageBox(QMessageBox::Critical, "Error",
                    "Selected directory doest not exist").exec();
        return;
    }
    if (QFile::exists(file_options->getFullFilename()))
    {
        QMessageBox(QMessageBox::Critical, "Error",
                    "File with this name already exists in " + file_options->getDirectory()).exec();
        return;
    }

    QScopedPointer<FFmpeg> ffmpeg(new FFmpeg);
    QScopedPointer<QThread> thread(new QThread);

    ffmpeg->setFrameSize(getSize());
    ffmpeg->setFiles(input_filename, file_options->getFullFilename());
    ffmpeg->setTime(t_start, t_end);
    ffmpeg->setFps(getFps());
    ffmpeg->setBehavior(getTransformation());

    WaitDialog dlg;

    connect(&dlg, SIGNAL(finished(int)), this, SLOT(close()));
    // Lambda is called from current thread, terminating ffmpeg located in another thread
    connect(&dlg, &WaitDialog::buttonCancel_clicked, [&thread, &ffmpeg, &dlg](){
        if (thread->isRunning())
            ffmpeg->terminate();
    });
    connect(&dlg, &WaitDialog::buttonOk_clicked, [&dlg](){
        dlg.close();
    });
    connect(&dlg, &WaitDialog::buttonOpen_clicked, [this, &dlg](){
        dlg.close();
        result_file = file_options->getFullFilename();
    });

    connect(thread.data(), SIGNAL(started()), ffmpeg.data(), SLOT(start()));
    connect(ffmpeg.data(), SIGNAL(stateChanged(QString)), &dlg, SLOT(setStatusInfo(QString)));
    connect(ffmpeg.data(), SIGNAL(finished()), thread.data(), SLOT(quit()));
    connect(ffmpeg.data(), SIGNAL(finished(QStringList)), &dlg, SLOT(showResult(QStringList)));

    ffmpeg->moveToThread(thread.data());
    thread->start();

    dlg.exec();
}

void MakeMovieDialog::comboBoxTransform_textChanged(const QString &text)
{
    QComboBox *cbox = static_cast<QComboBox*>(sender());
    if (!cbox)
        return;

    int index = cbox->currentIndex();
    if (index == 0)
    {
        file_options->removeAllSuffixes(true);
    }
    else
    {
        file_options->removeAllSuffixes();
        file_options->addSuffix(text, true);
    }

    comboBoxTransform_updateTooltip();
}

void MakeMovieDialog::comboBoxTransform_updateTooltip()
{
    QComboBox *cbox = ui->comboBox_transform;
    QString text = cbox->currentText();

    if (text == "None")
        cbox->setToolTip("Normal behavior without any additional effects");
    else if (text == "Reverse")
        cbox->setToolTip("Reverse the movie");
    else if (text == "Loop")
        cbox->setToolTip("Make it cycled (concatenate with it's reversed copy)");
    else if (text == "Mirror")
        cbox->setToolTip("Rotate the movie by 180 degrees");
    else if (text == "Insert keyframes")
        cbox->setToolTip("Insert keyframes to the movie.\n"
                         "Keyframes will be inserted at every 0.1s from the beginning.\n"
                         "This option will help if:\n"
                         "-you got few seconds of audio without video in the start\n"
                         "-you are not satisfied with cut precision\n"
                         "Has no effect on GIF output");

    cbox->setToolTip(cbox->toolTip() + "\n\n"
                                       "Any transformations made to the video may lower it's quality");
}
