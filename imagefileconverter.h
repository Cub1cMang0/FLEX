#ifndef IMAGEFILECONVERTER_H
#define IMAGEFILECONVERTER_H

#include <QString>
class ImageFileConverter
{
    public:
        virtual ~ImageFileConverter() {}
        virtual bool convert_image(const QString &input_path, const QString &output_path, QString &error_message) = 0;
};

#endif //IMAGEFILECONVERTER
