#include "mainvideoconverter.h"
#include <QString>
#include <QProcess>
#include <QFileDialog>
#include <QProgressBar>

MainVideoConverter::MainVideoConverter(QObject *parent)
    : VideoFileConverter(), QObject(parent) {}

bool MainVideoConverter::convert_video(const QString &input_path, const QString &output_path, QString &error_message, qint64 duration_ms)
{
    QStringList arguments;
    arguments << "-i" << input_path << "-progress" << "-" << "-y" << output_path;
    QProcess ffmpegProcess;
    QObject::connect(&ffmpegProcess, &QProcess::readyReadStandardOutput, [&ffmpegProcess, this, duration_ms]()
    {
        QByteArray output = ffmpegProcess.readAllStandardOutput();
        QList<QByteArray> lines = output.split('\n');
        for (const QByteArray &line : lines)
        {
            if (line.startsWith("out_time="))
            {
                QString timeStr = QString::fromUtf8(line.mid(9).trimmed());
                QTime time = QTime::fromString(timeStr.left(8), "hh:mm:ss");
                qint64 elapsed_ms = QTime(0, 0).msecsTo(time);
                int percent = duration_ms > 0 ? static_cast<int>((elapsed_ms * 100) / duration_ms) : 0;
                emit update_av_progress(qBound(0, percent, 100));
            }
        }
    });
    ffmpegProcess.start("ffmpeg", arguments);
    if (!ffmpegProcess.waitForStarted())
    {
        emit update_av_progress(qBound(0, 0, 100));
        error_message = "Failed to load video converter";
        return false;
    }
    if (!ffmpegProcess.waitForFinished(-1))
    {
        emit update_av_progress(qBound(0, 0, 100));
        error_message = "Failed to convert video file";
        return false;
    }
    int exit_code = ffmpegProcess.exitCode();
    if (exit_code != 0)
    {
        emit update_av_progress(qBound(0, 0, 100));
        error_message = "File could not be converted";
        return false;
    }
    else
    {
        emit update_av_progress(qBound(0, 100, 100));
        return true;
    }
}

qint64 get_file_duration(const QString &file_path)
{
    QProcess ffprobe;
    QStringList args = {
        "-v", "error",
        "-show_entries", "format=duration",
        "-of", "default=noprint_wrappers=1:nokey=1",
        file_path
    };

    ffprobe.start("ffprobe", args);
    if (!ffprobe.waitForFinished())
        return 0;
    QString ffprobe_output = ffprobe.readAllStandardOutput().trimmed();
    bool ok;
    double seconds = ffprobe_output.toDouble(&ok);
    return ok ? static_cast<qint64>(seconds * 1000) : 0;
}


QString convert_video_file(QString input_extension, QString output_extension, QProgressBar *progress_bar, bool alt_save_location)
{
    int duration_ms;
    QString input_info = input_extension + " Files " + "(*." + input_extension.toLower() + ")";
    QString file_path = QFileDialog::getOpenFileName(NULL, "Open File", "", input_info);
    QFileInfo input_file_info(file_path);
    QString output_name = input_file_info.completeBaseName() + "." + output_extension.toLower();
    QString output_path;
    if (alt_save_location)
    {
        QString output_info = output_extension + " Files " + "(*." + output_extension.toLower();
        output_path = QFileDialog::getSaveFileName(NULL, "Save File", "(*.", output_info);
    }
    else
    {
        QDir output_dir("output");
        output_path = output_dir.filePath(output_name);
    }
    QString result_message;
    MainVideoConverter *converter = new MainVideoConverter(progress_bar);
    qint64 file_duration = get_file_duration(file_path);
    QObject::connect(converter, &MainVideoConverter::update_av_progress, progress_bar, &QProgressBar::setValue);
    if (!converter->convert_video(file_path, output_path, result_message, file_duration)) {}
    else
    {
        result_message = "Success: " + input_file_info.completeBaseName() + "." + input_extension.toLower() + " has been converted to " + output_name;
    }
    delete converter;
    converter = nullptr;
    return result_message;
}
