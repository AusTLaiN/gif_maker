#ifndef FILEOPTIONS_H
#define FILEOPTIONS_H

#include <QWidget>

namespace Ui {
class FileOptions;
}

class FileOptions : public QWidget
{
    Q_OBJECT

public:
    explicit FileOptions(QWidget *parent = 0);
    ~FileOptions();

    QString getDirectory() const;
    QString getExtension() const;
    QString getFilename() const;
    QString getFullFilename() const;
    QString getFilenameWithSuffixes() const;
    QStringList getAvailableExtensions() const;

signals:
    void filenameChanged(const QString &filename);
    void directoryChanged(const QString &directory);
    void extensionChanged(const QString &extension);

public slots:
    void setAvailableExtensions(const QStringList &extensions);
    void setFileName(const QString &filename);
    void setDirectory(const QString &directory);
    void setExtension(const QString &extension);

    void addSuffix(const QString &suffix, bool check_name = false);
    void removeSuffix(const QString &suffix, bool check_name = false);
    void removeAllSuffixes(bool check_name = false);

    void checkFilename();

protected slots:
    void clickedBrowse();
    void comboBoxExtension_TextChanged();
    void lineEditFilename_TextChanged();

protected:
    QString filename;
    QString number; // Suffix that indicates number of same filenames in folder
    QStringList suffixes;

private:
    Ui::FileOptions *ui;
};

#endif // FILEOPTIONS_H
