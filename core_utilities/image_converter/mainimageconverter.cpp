#include "mainimageconverter.h"
#include "json.hpp"
#include <fstream>
#include <QFileDialog>
#include <QImage>
#include <QImageReader>
#include <QImageWriter>
#include <QPainter>

using json = nlohmann::json;
using namespace std;

MainImageConverter::MainImageConverter(QObject *parent)
    : ImageFileConverter(), QObject(parent) {}

void MainImageConverter::convert_image(const QString &input_path, const QString &output_path, const QString &input_ext, const QString &output_ext)
{
    QFileInfo input_file_info(input_path);
    QString output_name = input_file_info.completeBaseName() + "." + output_ext.toLower();
    QString complete_output = QDir(output_path).filePath(output_name);
    const auto &capabilities = image_capabilities[output_ext.toLower()];
    QImage image;
    QImageReader reader(input_path);
    if (!reader.canRead())
    {
        QString error = QString("Image could not be loaded: %1").arg(reader.errorString());
        emit update_image_progress(error, false);
        return;
    }
    image = reader.read();
    if (image.isNull())
    {
        QString error = QString("Image loading has failed: %1").arg(reader.errorString());
        emit update_image_progress(error, false);
        return;
    }
    QString app_dir = QCoreApplication::applicationDirPath();
    QString json_path = app_dir + "/conversion_preferences.json";
    ifstream save_json(json_path.toStdString());
    int image_quality = -1;
    if (!save_json.is_open())
    {
        QImageWriter writer(complete_output);
        writer.setFormat(output_ext.toLower().toUtf8());
        if (!writer.write(image))
        {
            QString error_msg = QString("Image could not be converted: %1").arg(writer.errorString());
            emit update_image_progress(error_msg, false);
            return;
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
                if (current_ratio > target_ratio)
                {
                    new_height = static_cast<int>(width / target_ratio);
                }
                else if (current_ratio < target_ratio)
                {
                    new_width = static_cast<int>(height * target_ratio);
                }
                QImage padded(new_width, new_height, image.format());
                padded.fill(Qt::transparent);
                QPainter painter(&padded);
                int x = (new_width - width) / 2;
                int y = (new_height - height) / 2;
                painter.drawImage(x, y, image);
                painter.end();
                image = padded;
            }
            if (capabilities.grayscale_support && image_preferences["grayscale"][0])
            {
                image = image.convertToFormat(QImage::Format_Grayscale8);
            }
            if (capabilities.alpha_support && image.hasAlphaChannel() && image_preferences["alpha"][0])
            {
                image = image.convertToFormat(QImage::Format_RGB32);
            }
            if (capabilities.bit_depth_support && image_preferences["bitdepth"][0])
            {
                int bit_depth = (QString::fromStdString(image_preferences["bitdepth"][1])).toInt();
                if (bit_depth == 1)
                {
                    image = image.convertToFormat(QImage::Format_Mono);
                }
                else if (bit_depth == 8)
                {
                    image = image.convertToFormat(QImage::Format_Indexed8);
                }
                else if (bit_depth == 24)
                {
                    image = image.convertToFormat(QImage::Format_RGB888);
                }
                else if (bit_depth == 32)
                {
                    image = image.convertToFormat(QImage::Format_RGB32);
                }
            }
            QString quality = QString::fromStdString(image_preferences["quality"][1]);
            if (capabilities.quality_support && quality != "None")
            {
                image_quality = quality.toInt();
            }
        }
    }
    QImageWriter writer(complete_output);
    writer.setFormat(output_ext.toLower().toUtf8());
    if (image_quality != -1)
    {
        writer.setQuality(image_quality);
    }
    if (!writer.write(image))
    {
        qDebug() << "B";
        QString error_msg = QString("Image could not be converted: %1").arg(writer.errorString());
        qDebug() << error_msg;
        emit update_image_progress(error_msg, false);
        return;
    }
    QString result = QString("Success: %1.%2 has been converted to %3")
        .arg(input_file_info.completeBaseName()).arg(input_ext.toLower()).arg(output_name);
    emit update_image_progress(result, true);
}
