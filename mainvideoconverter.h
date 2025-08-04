#ifndef MAINVIDEOCONVERTER_H
#define MAINVIDEOCONVERTER_H

#include <QImage>
#include <QObject>
#include <QProgressBar>
#include "videofileconverter.h"

class MainVideoConverter : public QObject, public VideoFileConverter
{
    Q_OBJECT
    public:
        explicit MainVideoConverter(QObject *parent = nullptr);
        bool convert_video(const QString &input_path, const QString &output_path, QString &error_message, qint64 duration_ms) override;
    signals:
        void update_av_progress(int percentage);
};

QString convert_video_file(QString input_extension, QString output_extension, QProgressBar *progress_bar, bool alt_save_location);

#endif // MAINVIDEOCONVERTER_H
