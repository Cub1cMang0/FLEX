#include "mainspreadconverter.h"
#include <QFileDialog>
#include <QDir>
#include <QProcess>
#include <QString>

MainSpreadConverter::MainSpreadConverter(QObject *parent)
    : QObject(parent)
{}

void MainSpreadConverter::convert_spread(const QString &input_path, const QString &output_path, QString input_extension, QString output_extension)
{
    QStringList arguments;
    arguments << "--headless" << "--convert-to" << output_extension.toLower() << input_path << "--outdir" << output_path;
    soffice = new QProcess(this);
    connect(soffice, &QProcess::errorOccurred, this, [=](QProcess::ProcessError error)
    {
        Q_UNUSED(error);
        emit conversion_finished(false, "Failed to load spreadsheet converter.");
        soffice->deleteLater();
    });
    connect(soffice, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [=](int exitCode, QProcess::ExitStatus status)
    {
        if (status != QProcess::NormalExit || exitCode != 0)
        {
            if (output_path != "") {emit conversion_finished(false, "File could not be converter");}
            else {emit conversion_finished(false, "No location selected");}
        }
        else
        {
            QFileInfo input_file_info(input_path);
            QString result = "Success: " + input_file_info.completeBaseName() + "." + input_extension.toLower() + " has been converter to " + input_file_info.completeBaseName() + "." + output_extension.toLower();
            emit conversion_finished(true, result);
        }
        soffice->deleteLater();
    });
    soffice->start("soffice", arguments);
}

void convert_spread_file(QWidget *parent, QString input_extension, QString output_extension, MainSpreadConverter *converter, QString save_folder)
{
    QString input_info = input_extension + " Files " + "(*." + input_extension.toLower() + ")";
    QString file_path = QFileDialog::getOpenFileName(NULL, "Open File", "", input_info);
    QString output_path;
    if (save_folder == "Alternate")
    {
        QString output_info = output_extension + " Files " + "(*." + output_extension.toLower() + ")";
        output_path = QFileDialog::getSaveFileName(NULL, "Save File", "", output_info);
    }
    else
    {
        QDir output_dir(save_folder);
        output_path = output_dir.path();
    }
    converter->convert_spread(file_path, output_path, input_extension, output_extension);
}
