#include "mainimageconverter.h"
#include <QFileDialog>

bool MainImageConverter::convert_image(const QString &input_path, const QString &output_path, QString &error_message)
{
    QImage image;
    if (!image.load(input_path))
    {
        error_message = "Failed to load image.";
        return false;
    }
    if (!image.save(output_path))
    {
        error_message = "Failed to save image.";
        return false;
    }
    return true;
}

// Used for image file conversion (so far, it only supports the base image read and write of QImageReader/Writer)
QString convert_image_file(QString input_extension, QString output_extension, bool alt_save_location)
{
    QString input_info = input_extension + " Files " + "(*." + input_extension.toLower() + ")";
    QString file_path = QFileDialog::getOpenFileName(NULL, "Open File", "", input_info);
    QFileInfo input_file_info(file_path);
    QString output_name = input_file_info.completeBaseName() + "." + output_extension.toLower();
    QString output_path;
    if (alt_save_location)
    {
        QString output_info = output_extension + " Files " + "(*." + output_extension.toLower() + ")";
        output_path = QFileDialog::getSaveFileName(NULL, "Save File", "", output_info);
        if (output_path.isEmpty())
        {
            return "No location selected";
        }
    }
    else
    {
        QDir output_dir("output");
        output_path = output_dir.filePath(output_name);
    }
    QString error_message;
    MainImageConverter converter;
    if (!converter.convert_image(file_path, output_path, error_message))
    {
        return error_message;
    }
    else
    {
        QString success_message = "Success: " + input_file_info.completeBaseName() + "." + input_extension.toLower() + " has been converted to " + output_name;
        return success_message;
    }
}
