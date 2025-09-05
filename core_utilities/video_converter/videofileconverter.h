#ifndef VIDEOFILECONVERTER_H
#define VIDEOFILECONVERTER_H

#include <QString>
class VideoFileConverter
{
    public:
        virtual ~VideoFileConverter() {}
        bool convert_video(const QString &input_path, const QString &output_path, QString &error_message, qint64 duration_ms);
};

#endif // VIDEOFILECONVERTER_H
