#ifndef MAINIMAGECONVERTER_H
#define MAINIMAGECONVERTER_H

#include <QImage>
#include "imagefileconverter.h"

class MainImageConverter: ImageFileConverter
{
    public:
        bool convert_image(const QString &input_path, const QString &output_path, QString &error_message) override;
};

QString convert_image_file(QString input_extension, QString output_extension);

#endif
