#ifndef VIDEOFILECONVERTER_H
#define VIDEOFILECONVERTER_H

#include <QString>
class VideoFileConverter
{
    public:
        virtual ~VideoFileConverter() {}
        virtual bool convert_video(const QString &input_path, const QString &output_path, QString &error_message, qint64 duration_ms) = 0;
};

#endif // VIDEOFILECONVERTER_H
