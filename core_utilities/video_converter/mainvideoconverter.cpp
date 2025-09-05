#include "mainvideoconverter.h"
#include "json.hpp"
#include <fstream>
#include <QString>
#include <QProcess>
#include <QFileDialog>
#include <QProgressBar>

using json = nlohmann::json;
using namespace std;

MainVideoConverter::MainVideoConverter(QObject *parent)
    : VideoFileConverter(), QObject(parent) {}

QSet<QString> video_formats = {"MP4", "MOV", "AVI", "WMV", "MKV", "M4V"};



QString set_resolution(QString resolution)
{
    QString res = "";
    if (resolution == "144p") {res = "256x144";}
    if (resolution == "240p") {res = "426x240";}
    if (resolution == "360p") {res = "640x360";}
    if (resolution == "480p") {res = "854x480";}
    if (resolution == "720p") {res = "1280x720";}
    if (resolution == "1080p") {res = "1920x1080";}
    if (resolution == "1440p") {res = "2560x1440";}
    if (resolution == "1800p") {res = "3200x1800";}
    if (resolution == "2160p") {res = "3840x2160";}
    return res;
}

double codec_factor(QString codec)
{
    double factor = 0.0;
    if (codec == "libx264") {factor = 1.0;}
    if (codec == "libx265") {factor = 0.65;}
    if (codec == "libvpx-vp9") {factor = 0.6;}
    if (codec == "libaom-av1") {factor = 0.55;}
    return factor;
}

QString set_encoder_name(QString encoder)
{
    QString proper_encoder = "";
    if (encoder == "H.264") {proper_encoder = "libx264";}
    if (encoder == "H.265") {proper_encoder = "libx265";}
    if (encoder == "VP9") {proper_encoder = "libvpx-vp9";}
    if (encoder == "AV1") {proper_encoder = "libaom-av1";}
    if (encoder == "FFV1") {proper_encoder = "ffv1";}
    return proper_encoder;
}

double bppf_factor(QString level)
{
    double factor = 0.0;
    if (level == "Low") {factor = 0.035;}
    if (level == "Medium") {factor = 0.060;}
    if (level == "High") {factor = 0.090;}
    if (level == "Very High") {factor = 0.120;}
    return factor;
}

QString set_audio_channel(QString channel)
{
    QString selected_channel = "";
    if (channel == "Mono") {selected_channel = "1";}
    if (channel == "Stereo") {selected_channel = "2";}
    if (channel == "5.1 Surround") {selected_channel = "6";}
    if (channel == "7.1 Surround") {selected_channel = "8";}
    return selected_channel;
}

bool MainVideoConverter::convert_video(const QString &input_path, const QString &output_path, QString input_extension, QString output_extension, QString &error_message, qint64 duration_ms)
{
    QStringList arguments;
    arguments << "-y" << "-i" << input_path << "-progress" << "pipe:1";
    QProcess ffmpegProcess;
    QString source_location = QString(__FILE__);
    QFileInfo file_info(source_location);
    QString cpp_directory = file_info.absolutePath();
    QString json_path = cpp_directory + "/conversion_preferences.json";
    ifstream save_json(json_path.toStdString());
    if (save_json.is_open())
    {
        json load_data;
        save_json >> load_data;
        save_json.close();
        auto videoaudio_preferences = load_data["video_audio"];
        QString resolution = QString::fromStdString(videoaudio_preferences["resolution"]);
        resolution = set_resolution(resolution);
        QString frame_rate = QString::fromStdString(videoaudio_preferences["framerate"]);
        frame_rate = frame_rate.left(frame_rate.length() - 4);
        QString video_bitrate = QString::fromStdString(videoaudio_preferences["video_bitrate"]);
        QString video_codec = QString::fromStdString(videoaudio_preferences["video_codec"]);
        video_codec = set_encoder_name(video_codec);
        QString sample_rate = QString::fromStdString(videoaudio_preferences["sample_rate"]);
        QString channel = QString::fromStdString(videoaudio_preferences["channel"]);
        QString audio_bitrate = QString::fromStdString(videoaudio_preferences["audio_bitrate"]);
        audio_bitrate = audio_bitrate.left(audio_bitrate.length() - 5) + "k";
        QString audio_codec = QString::fromStdString(videoaudio_preferences["audio_codec"]);
        if (video_formats.contains(input_extension) && video_formats.contains(output_extension))
        {
            if (resolution != "N/A") {arguments << "-s" << resolution;}
            if (frame_rate != "N/A") {arguments << "-r" << frame_rate;}
            if (video_bitrate != "N/A" && video_codec != "FFV1")
            {
                int x_location = resolution.indexOf('x');
                double bitrate_calculation = ((resolution.left(x_location)).toInt() * (resolution.mid(x_location + 1)).toInt() *
                                              frame_rate.toInt() * bppf_factor(video_bitrate) * codec_factor(video_codec));
                QString final_bitrate = QString::number((int((bitrate_calculation + 50) / 100) * 100));
                arguments << "-b:v" << final_bitrate;
            }
            if (video_codec != "N/A") {arguments << "-c:v" << video_codec;}
        }
        if (sample_rate != "N/A") {arguments << "-ar" << sample_rate.left(sample_rate.length() - 3);}
        if (channel != "N/A") {arguments << "-ac" << set_audio_channel(channel);}
        if (audio_bitrate != "N/A") {arguments << "-b:a" << audio_bitrate;}
        if (audio_codec != "N/A") {arguments << "-c:a" << audio_codec.toLower();}
        arguments << output_path;
    }
    else
    {
        return false;
    }
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
        QByteArray error = ffmpegProcess.readAllStandardError();
        qDebug() << error;
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


QString convert_video_file(QString input_extension, QString output_extension, QProgressBar *progress_bar, QString save_folder)
{
    int duration_ms;
    QString input_info = input_extension + " Files " + "(*." + input_extension.toLower() + ")";
    QString file_path = QFileDialog::getOpenFileName(NULL, "Open File", "", input_info);
    if (file_path.isEmpty())
    {
        return "No video or audio file selected";
    }
    QFileInfo input_file_info(file_path);
    QString output_name = input_file_info.completeBaseName() + "." + output_extension.toLower();
    QString output_path;
    if (save_folder == "Alternate")
    {
        QString output_info = output_extension + " Files " + "(*." + output_extension.toLower();
        output_path = QFileDialog::getSaveFileName(NULL, "Save File", "(*.", output_info);
        if (output_path.isEmpty())
        {
            return "No location selected";
        }
    }
    else
    {
        QDir output_dir(save_folder);
        output_path = output_dir.filePath(output_name);
    }
    QString result_message;
    MainVideoConverter *converter = new MainVideoConverter(progress_bar);
    qint64 file_duration = get_file_duration(file_path);
    QObject::connect(converter, &MainVideoConverter::update_av_progress, progress_bar, &QProgressBar::setValue);
    if (!converter->convert_video(file_path, output_path, input_extension, output_extension, result_message, file_duration)) {}
    else
    {
        result_message = "Success: " + input_file_info.completeBaseName() + "." + input_extension.toLower() + " has been converted to " + output_name;
    }
    delete converter;
    converter = nullptr;
    return result_message;
}
