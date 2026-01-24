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

void MainVideoConverter::convert_video(const QString &input_path, const QString &output_path, QString input_extension, QString output_extension)
{
    QStringList arguments;
    arguments << "-y" << "-i" << input_path << "-progress" << "pipe:1";
    ffmpeg_process = new QProcess(this);
    ffmpeg_process->setProgram("ffmpeg");
    QString source_location = QString(__FILE__);
    QFileInfo file_info(source_location);
    QString cpp_directory = file_info.absolutePath();
    QString json_path = cpp_directory + "/conversion_preferences.json";
    ifstream save_json(json_path.toStdString());
    json load_data;
    if (save_json.is_open())
    {
        save_json >> load_data;
        save_json.close();
    }
    if (load_data.contains("videoaudio"))
    {
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
    }
    arguments << output_path;
    ffmpeg_process->setArguments(arguments);
    connect(ffmpeg_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [=](int exitCode, QProcess::ExitStatus status)
    {
        if (exitCode != 0)
        {
            QByteArray error = ffmpeg_process->readAllStandardError();
            emit update_av_progress(0);
        }
        else
        {
            emit update_av_progress(100);
        }
        ffmpeg_process->deleteLater();
    });
    ffmpeg_process->start();
}

void MainVideoConverter::convert_video_file(QString file_path, QString input_extension, QString output_extension, QString save_folder)
{
    if (file_path.isEmpty())
    {
        emit update_result_message("No video/audio file selected", false);
        return;
    }
    QFileInfo input_file_info(file_path);
    QString output_name = input_file_info.completeBaseName() + "." + output_extension.toLower();
    QDir output_dir(save_folder);
    QString output_path = output_dir.filePath(output_name);
    auto *converter = new MainVideoConverter(this);
    connect(converter, &MainVideoConverter::update_result_message, this, [=](bool success, const QString &error)
    {
        QString result_message;
        if (success)
        {
            result_message = QString("Success: %1.%2 has been converted to %3")
                .arg(input_file_info.completeBaseName()).arg(input_extension.toLower()).arg(output_name);
        }
        else
        {
            result_message = error.isEmpty() ? "File could not be converted" : error;
        }
        emit update_result_message(result_message, success);
        converter->deleteLater();
    });
    converter->convert_video(file_path, output_path, input_extension, output_extension);
}
