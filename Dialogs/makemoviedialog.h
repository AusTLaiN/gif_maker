#ifndef MAKEMOVIEDIALOG_H
#define MAKEMOVIEDIALOG_H

#include "Widgets/fileoptions.h"
#include "Base/ffmpeg.h"

#include <QDialog>
#include <QTime>

namespace Ui {
class MakeMovieDialog;
}

class MakeMovieDialog : public QDialog
{
    Q_OBJECT

public:
    QString getInputFilename() const;
    void setInputFilename(const QString &filename);

public:
    explicit MakeMovieDialog(QWidget *parent = 0);
    MakeMovieDialog(const QString &input_file_fullname, QWidget *parent = nullptr);
    ~MakeMovieDialog();

    FFmpeg::Behavior getTransformation() const;
    QSize getSize() const;
    int getFps() const;
    QString getResult() const;

public slots:
    void setTime(const QTime &start, const QTime &end, const QTime &total);
    void allowTransformation(bool allowed);
    void createMovie();

protected slots:
    void comboBoxTransform_textChanged(const QString &text);
    void comboBoxTransform_updateTooltip();

protected:
    FileOptions *file_options;
    QTime t_start, t_end, t_total;
    QString input_filename, result_file;

private:
    Ui::MakeMovieDialog *ui;
};

#endif // MAKEMOVIEDIALOG_H
