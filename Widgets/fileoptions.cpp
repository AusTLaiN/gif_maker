#include "fileoptions.h"
#include "ui_fileoptions.h"

#include <QStyle>
#include <QFileDialog>
#include <QSettings>
#include <QDebug>
#include <QStringBuilder>


FileOptions::FileOptions(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FileOptions)
{
    ui->setupUi(this);

    ui->button_browse->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));   
    ui->button_browse->setToolTip("Select where new files should be saved");
    ui->button_browse->setCursor(Qt::PointingHandCursor);
    ui->comboBox_extension->setToolTip("Select output file extension");
    ui->lineEdit_filename->setToolTip("Type only filename here, without extension");


    connect(ui->comboBox_extension, SIGNAL(currentTextChanged(QString)), this, SIGNAL(extensionChanged(QString)));
    connect(ui->lineEdit_filename, &QLineEdit::editingFinished, [this](){
        emit filenameChanged(ui->lineEdit_filename->text());
    });
    connect(ui->lineEdit_directory, &QLineEdit::editingFinished, [this](){
        emit filenameChanged(ui->lineEdit_directory->text());
    });

    connect(ui->button_browse, SIGNAL(clicked(bool)), this, SLOT(clickedBrowse()));
    connect(ui->comboBox_extension, SIGNAL(currentTextChanged(QString)), this, SLOT(comboBoxExtension_TextChanged()));
    connect(ui->lineEdit_filename, SIGNAL(editingFinished()), this, SLOT(lineEditFilename_TextChanged()));


    QStringList knownExtensions = QStringList {
            ".avi", ".gif", ".mkv", ".mp4", ".wmv"
        };
    setAvailableExtensions(knownExtensions);

    QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       QCoreApplication::organizationName() , QCoreApplication::applicationName());
    QString dirname = settings.value("Working directory", "Select directory").toString();
    setDirectory(dirname);
}

FileOptions::~FileOptions()
{
    delete ui;
}

QString FileOptions::getDirectory() const
{
    return ui->lineEdit_directory->text();
}

QString FileOptions::getExtension() const
{
    return ui->comboBox_extension->currentText();
}

QString FileOptions::getFilename() const
{
    return filename;
}

QString FileOptions::getFullFilename() const
{
    return getDirectory() % "/" % getFilenameWithSuffixes() % getExtension();
}

QString FileOptions::getFilenameWithSuffixes() const
{
    QString complete_filename = filename;

    if (!suffixes.isEmpty())
        complete_filename += "_" + suffixes.join("_");
    if (number != "")
        complete_filename += "_" + number;

    return complete_filename;
}

void FileOptions::setAvailableExtensions(const QStringList &extensions)
{
    ui->comboBox_extension->clear();
    ui->comboBox_extension->addItems(extensions);
}

void FileOptions::setFileName(const QString &filename)
{
    this->filename = filename;
    number = "";
    checkFilename();
}

void FileOptions::setDirectory(const QString &directory)
{
    ui->lineEdit_directory->setText(directory);
    number = "";
    checkFilename();
}

void FileOptions::setExtension(const QString &extension)
{
    ui->comboBox_extension->setCurrentText(extension);
    number = "";
    checkFilename();
}

void FileOptions::addSuffix(const QString &suffix, bool check_name)
{
    suffixes << suffix;
    this->number = "";

    if (check_name)
        checkFilename();
}

void FileOptions::removeSuffix(const QString &suffix, bool check_name)
{
    suffixes.removeAll(suffix);
    number = "";

    if (check_name)
        checkFilename();
}

void FileOptions::removeAllSuffixes(bool check_name)
{
    suffixes.clear();
    number = "";

    if (check_name)
        checkFilename();
}

void FileOptions::checkFilename()
{
    qDebug() << "FileOptions::checkFilename: filename = " << getFullFilename();

    int i = 1;
    do
    {
        if (!QFile(getFullFilename()).exists())
        {
            ui->lineEdit_filename->setText(getFilenameWithSuffixes());
            break;
        }

        number = QString::number(++i);
    }
    while(true);
}

QStringList FileOptions::getAvailableExtensions() const
{
    QStringList list;

    for (int i = 0; i < ui->comboBox_extension->count(); ++i)
    {
        list.append(ui->comboBox_extension->itemText(i));
    }

    return list;
}

void FileOptions::clickedBrowse()
{
    QFileDialog dialog(this);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOptions(QFileDialog::ShowDirsOnly);
    dialog.setDirectory(getDirectory());
    dialog.setWindowTitle("Select folder");

    if (dialog.exec() == QDialog::Accepted)
    {
        auto dirname = dialog.selectedFiles().first();
        setDirectory(dirname);

        QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                           QCoreApplication::organizationName() , QCoreApplication::applicationName());
        settings.setValue("Working directory", dirname);
    }
}

void FileOptions::comboBoxExtension_TextChanged()
{
    number = "";
    checkFilename();
}

void FileOptions::lineEditFilename_TextChanged()
{
    removeAllSuffixes();
    filename = ui->lineEdit_filename->text();
}
