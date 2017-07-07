#include "makemoviedialog.h"
#include "ui_makemoviedialog.h"
#include "waitdialog.h"

#include <QMessageBox>
#include <QThread>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QMetaEnum>

MakeMovieDialog::MakeMovieDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MakeMovieDialog),
    file_options(new FileOptions)
{
    ui->setupUi(this);
    // Hide "?" button
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    setWindowTitle("Movie options");


    ui->verticalLayout_1->addWidget(file_options);

    ui->comboBox_height->setToolTip("Output movie height.\n"
                                    "Scale works only if another dimension set.\n"
                                    "Scaling both dimensions will give the original size");
    ui->comboBox_width->setToolTip("Output movie width.\n"
                                    "Scale works only if another dimension set.\n"
                                    "Scaling both dimensions will give the original size");
    ui->comboBox_fps->setToolTip("Output movie fps.\n"
                                 "Fps cannot exceed original value");


    connect(ui->button_create, SIGNAL(clicked(bool)), SLOT(createMovie()));
    connect(ui->comboBox_transform, SIGNAL(currentTextChanged(QString)), SLOT(comboBoxTransform_textChanged(QString)));

    // Validations

    connect(ui->comboBox_width, SIGNAL(editTextChanged(QString)), SLOT(validateTextEditing(QString)));
    connect(ui->comboBox_height, SIGNAL(editTextChanged(QString)), SLOT(validateTextEditing(QString)));
    connect(ui->comboBox_fps, SIGNAL(editTextChanged(QString)), SLOT(validateTextEditing(QString)));
    connect(file_options, SIGNAL(extensionChanged(QString)), SLOT(validateTransformations()));

    // Set available transformations

    QMetaEnum metaEnum = QMetaEnum::fromType<FFmpeg::Transform>();
    QStringList availableTransforms;

    for (int i = 0; i < metaEnum.keyCount(); ++i)
        availableTransforms.append(metaEnum.key(i));

    ui->comboBox_transform->clear();
    ui->comboBox_transform->addItems(availableTransforms);

    // Available sizes

    QStringList sizes;
    sizes << "Original";
    sizes << "Scale";
    sizes << "1080";
    sizes << "720";
    sizes << "640";
    sizes << "480";
    sizes << "320";
    sizes << "240";

    ui->comboBox_width->addItems(sizes);
    ui->comboBox_height->addItems(sizes);

    // Available fps values

    QStringList fps;
    fps << "60"
        << "50"
        << "40"
        << "30"
        << "20"
        << "15"
        << "10"
        << "5";

    ui->comboBox_fps->addItems(fps);

    // Default values

    resize(440, 200);

    ui->comboBox_fps->setCurrentText("20");
    ui->comboBox_width->setCurrentText("Original");
    ui->comboBox_height->setCurrentText("Original");
    comboBoxTransform_updateTooltip();

    ui->button_create->setCursor(Qt::PointingHandCursor);
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

    input_extension = extension;

    file_options->setFileName(name_only);
    file_options->setExtension(".gif");
}

MakeMovieDialog::~MakeMovieDialog()
{
    delete ui;
}

FFmpeg::Transform MakeMovieDialog::getTransformation() const
{
    QMetaEnum metaEnum = QMetaEnum::fromType<FFmpeg::Transform>();
    int index = ui->comboBox_transform->currentIndex();

    return static_cast<FFmpeg::Transform>(metaEnum.value(index));
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

    validateTransformations();
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

    // Lambda is called from current thread, terminating ffmpeg located in another thread
    connect(&dlg, &WaitDialog::buttonCancel_clicked, [&thread, &ffmpeg, &dlg](){
        if (thread->isRunning())
            ffmpeg->terminate();
    });

    connect(thread.data(), SIGNAL(started()), ffmpeg.data(), SLOT(start()));
    connect(ffmpeg.data(), SIGNAL(stateChanged(QString)), &dlg, SLOT(setStatusInfo(QString)));
    connect(ffmpeg.data(), SIGNAL(finished()), thread.data(), SLOT(quit()));
    connect(ffmpeg.data(), SIGNAL(finished(QStringList)), &dlg, SLOT(showResult(QStringList)));

    ffmpeg->moveToThread(thread.data());
    thread->start();
    int res = dlg.exec();

    switch(res) {
    case WaitDialog::Open:
        result_file = file_options->getFullFilename();
        break;
    case WaitDialog::Show:
    {
        QStringList args;
        auto file = QDir::toNativeSeparators(file_options->getFullFilename());
        args << "/select," << file;

        QProcess::startDetached("explorer.exe", args);
        break;
    }
    case WaitDialog::Ok:
        break;
    default:
        qDebug("MakeMovieDialog::createMovie: Unknown WaitDialogResult %i", res);
    }

    done(res);
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
    QMetaEnum metaEnum = QMetaEnum::fromType<FFmpeg::Transform>();

    QString text = cbox->currentText();    
    std::string key = text.toStdString();
    FFmpeg::Transform value = static_cast<FFmpeg::Transform>(metaEnum.keyToValue(key.c_str()));

    //qDebug() << "MakeMovieDialog::comboBoxTransform_updateTooltip: Value = " << value;

    switch(value) {
    case FFmpeg::None:
        cbox->setToolTip("Normal behavior without any additional effects");
        break;
    case FFmpeg::Reverse:
        cbox->setToolTip("Reverse the movie");
        break;
    case FFmpeg::Mirror:
        cbox->setToolTip("Rotate the movie by 180 degrees");
        break;
    case FFmpeg::Loop:
        cbox->setToolTip("Make it cycled (concatenate with it's reversed copy)");
        break;
    case FFmpeg::InsertKeyframes:
        cbox->setToolTip("Insert keyframes to the movie.\n"
                         "Has no effect on GIF output.\n"
                         "Keyframes will be inserted every 0.1s from the beginning.\n"
                         "This option will help if:\n"
                         "-you got few seconds of audio without video in the start\n"
                         "-you are not satisfied with cut precision");
        break;
    }

    cbox->setToolTip(cbox->toolTip() + "\n\n"
                                       "Any transformations made to the video may lower it's quality");
}

void MakeMovieDialog::validateTextEditing(const QString &text)
{
    auto cbox = static_cast<QComboBox*>(sender());

    if (text.contains(QRegExp("^[0-9]+$")))
        return;

    for (int i = 0; i < cbox->count(); ++i)
    {
        if (cbox->itemText(i) == text)
            return;
    }

    cbox->setCurrentText("");
}

void MakeMovieDialog::validateTransformations()
{
    if (t_start.msecsTo(t_end) != QTime(0, 0, 0).msecsTo(t_total))
    {
        ui->comboBox_transform->setEnabled(false);
        ui->comboBox_transform->setCurrentIndex(0);
        ui->comboBox_transform->setToolTip("Transformation can be applied only to the entire movie.\n"
                                           "Clear both markers to enable this option");
        return;
    }

    //qDebug() << "MakeMovieDialog::validateTransformations: Input fname extension = " << input_extension;

    if (file_options->getExtension() != input_extension)
    {
        ui->comboBox_transform->setEnabled(false);
        ui->comboBox_transform->setCurrentIndex(0);
        ui->comboBox_transform->setToolTip("Transformation and conversion to another format cannot be done together");
        return;
    }

    ui->comboBox_transform->setEnabled(true);
    comboBoxTransform_updateTooltip();
}
