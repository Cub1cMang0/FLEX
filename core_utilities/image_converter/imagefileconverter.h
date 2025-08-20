#ifndef IMAGEFILECONVERTER_H
#define IMAGEFILECONVERTER_H

#include <QString>
#include "imagepreferences.h"
class ImageFileConverter
{
    public:
        virtual ~ImageFileConverter() {}
        virtual bool convert_image(const QString &input_path, const QString &output_path, const ImageFormatCapabilities &settings, QString &error_message) = 0;
};

#endif //IMAGEFILECONVERTER
