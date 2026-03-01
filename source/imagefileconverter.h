#ifndef IMAGEFILECONVERTER_H
#define IMAGEFILECONVERTER_H

#include <QString>
#include "imagepreferences.h"
class ImageFileConverter
{
    public:
        virtual ~ImageFileConverter() {}
        bool convert_image(const QString &input_path, const QString &output_path, const ImageFormatCapabilities &settings, QString &error_message);
};

#endif //IMAGEFILECONVERTER
