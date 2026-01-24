#include "mainimageconverter.h"
#include "json.hpp"
#include <fstream>
#include <QFileDialog>
#include <QImage>
#include <QPainter>

using json = nlohmann::json;
using namespace std;

bool MainImageConverter::convert_image(const QString &input_path, const QString &output_path, const ImageFormatCapabilities &settings, QString &error_message)
{
    QImage image;
    if (!image.load(input_path))
    {
        error_message = "Failed to load image.";
        return false;
    }
    QString source_location = QString(__FILE__);
    QFileInfo file_info(source_location);
    QString cpp_directory = file_info.absolutePath();
    QString json_path = cpp_directory + "/conversion_preferences.json";
    ifstream save_json(json_path.toStdString());
    if (!save_json.is_open())
    {
        if (!image.save(input_path))
        {
            error_message = "Failed to save image.";
            return false;
        }
    }
    else
    {
        json load_data;
        save_json >> load_data;
        save_json.close();
        if (load_data.contains("image"))
        {
            auto image_preferences = load_data["image"];
            if (image_preferences["aspect_ratio"][0])
            {
                QString aspect_ratio = QString::fromStdString(image_preferences["aspect_ratio"][1]);
                int width = image.width();
                int height = image.height();
                double current_ratio = static_cast<double>(width) / height;
                int colon_location = aspect_ratio.indexOf(':');
                QString first_num = aspect_ratio.left(colon_location - 1);
                QString second_num = aspect_ratio.mid(colon_location + 2);
                double target_ratio = static_cast<double>(first_num.toInt()) / second_num.toInt();
                int new_width = width;
                int new_height = height;
                if (current_ratio > target_ratio) {new_height = static_cast<int>(width / target_ratio);}
                else if (current_ratio < target_ratio) {new_width = static_cast<int>(height * target_ratio);}
                QImage padded(new_width, new_height, image.format());
                padded.fill(Qt::transparent);
                QPainter painter(&padded);
                int x = (new_width - width) / 2;
                int y = (new_height - height) / 2;
                painter.drawImage(x, y, image);
                painter.end();
                image = padded;
            }
            if (settings.grayscale_support && image_preferences["grayscale"][0]) {image = image.convertToFormat(QImage::Format_Grayscale8);}
            if (settings.alpha_support && image.hasAlphaChannel() && image_preferences["alpha"][0]) {image = image.convertToFormat(QImage::Format_RGB32);}
            if (settings.bit_depth_support && image_preferences["bitdepth"][0])
            {
                int bit_depth = (QString::fromStdString(image_preferences["bitdepth"][1])).toInt();
                if (bit_depth == 1) {image = image.convertToFormat(QImage::Format_Mono);}
                else if (bit_depth == 8) {image = image.convertToFormat(QImage::Format_Indexed8);}
                else if (bit_depth == 24) {image = image.convertToFormat(QImage::Format_RGB888);}
                else if (bit_depth == 32) {image = image.convertToFormat(QImage::Format_RGB32);}
            }
            QString quality = QString::fromStdString(image_preferences["quality"][1]);
            if (settings.quality_support && quality != "None")
            {
                int image_quality = quality.toInt();
                if (!image.save(output_path, nullptr, image_quality))
                {
                    error_message = "Failed to save image.";
                    return false;
                }
            }
        }
    }
    if (!image.save(output_path))
    {
        error_message = "Failed to save iamge.";
        return false;
    }
    return true;
}

// Used for image file conversion (so far, it only supports the base image read and write of QImageReader/Writer)
QString convert_image_file(QString file_path, QString input_extension, QString output_extension, QString save_folder)
{
    if (file_path.isEmpty())
    {
        return "No image file selected";
    }
    QFileInfo input_file_info(file_path);
    QString output_name = input_file_info.completeBaseName() + "." + output_extension.toLower();
    QDir output_dir(save_folder);
    QString output_path = output_dir.filePath(output_name);
    QString error_message;
    MainImageConverter converter;
    const auto &capabilities = image_capabilities[output_extension.toLower()];
    if (!converter.convert_image(file_path, output_path, capabilities, error_message))
    {
        return error_message;
    }
    else
    {
        QString success_message = "Success: " + input_file_info.completeBaseName() + "." + input_extension.toLower() + " has been converted to " + output_name;
        return success_message;
    }
}
