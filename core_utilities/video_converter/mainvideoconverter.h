#ifndef MAINVIDEOCONVERTER_H
#define MAINVIDEOCONVERTER_H

#include <QImage>
#include <QObject>
#include <QProcess>
#include "videofileconverter.h"

class MainVideoConverter : public QObject, public VideoFileConverter
{
    Q_OBJECT
    public:
        explicit MainVideoConverter(QObject *parent = nullptr);
        void convert_video(const QString &input_path, const QString &output_path, QString input_extension, QString output_extension);
        void convert_video_file(QString file_path, QString input_extension, QString output_extension, QString save_folder);
    signals:
        void update_av_progress(int percentage);
        void update_result_message(const QString &message, bool success);
    private:
        QProcess *ffmpeg_process = nullptr;
};

#endif // MAINVIDEOCONVERTER_H
