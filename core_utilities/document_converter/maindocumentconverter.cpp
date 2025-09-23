#include "maindocumentconverter.h"
#include <QString>
#include <QProcess>
#include <QFileDialog>
#include <QDir>
#include <QFile>
#include <QWidget>
#include <QTemporaryFile>
#include <QMessageBox>
#include <sstream>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include "json.hpp"

using json = nlohmann::json;
using namespace std;

MainDocumentConverter::MainDocumentConverter(QObject *parent)
    : QObject(parent), pandoc(nullptr)
{}

string json_markdown_wrap(const json &j)
{
    stringstream ss;
    ss << "```json\n" << j.dump(4) << "\n```\n";
    return ss.str();
}

void save_new_json(const string &file_name, const string &data)
{
    ofstream output(file_name);
    if (!output)
    {
        throw runtime_error("Failed to open " + file_name);
    }
    output << data;
    output.close();
}

bool format_json(const QString &input_path, const QString &output_path)
{
    QStringList arguments;
    arguments << input_path << "-o" << output_path;
    QProcess pandoc;
    pandoc.start("pandoc", arguments);
    if (!pandoc.waitForStarted())
    {
        return false;
    }
    if (!pandoc.waitForFinished())
    {
        return false;
    }
    int exit_code = pandoc.exitCode();
    if (exit_code != 0)
    {
        return false;
    }
    return true;
}

void MainDocumentConverter::convert_document(const QString &input_path, const QString &output_path, QString input_extension, QString output_extension, const DocumentFormatCapabilities &settings)
{
    this->output_path = output_path;
    QStringList arguments;
    arguments << input_path << "-o" << output_path;
    QString temp_ref_path;
    pandoc = new QProcess(this);
    QString source_location = QString(__FILE__);
    QFileInfo file_info(source_location);
    QString cpp_directory = file_info.absolutePath();
    QString json_path = cpp_directory + "/conversion_preferences.json";
    ifstream save_json(json_path.toStdString());
    if (save_json.is_open())
    {
        json load_data;
        save_json >> load_data;
        save_json.close();
        auto document_preferences = load_data["document"];
        int rem_metadata = document_preferences["pres_metadata"];
        int pres_formatting = document_preferences["pres_formatting"];
        int pres_media = document_preferences["pres_media"];
        int line_break = document_preferences["line_break"];
        int standalone = document_preferences["standalone"];
        if (rem_metadata == 2 && settings.remove_meta_support) {arguments << "--metadata=title=" << "--metadata=author=" << "--metadata=date=";}
        if (pres_media == 2 && settings.preserve_media_support) {arguments << "--extract-media=media";}
        if (line_break == 2 && settings.line_break_support) {arguments << "--wrap=preserve";}
        if (standalone == 2 && settings.standalone_support) {arguments << "--standalone";}
        if (pres_formatting == 2 && settings.preserve_format_support)
        {
            QString base_name = "reference." + (output_extension == "PDF" ? "tex" : output_extension.toLower());
            temp_ref_path = QDir::currentPath() + "/" + base_name;
            QFile temp_ref_file(temp_ref_path);
            if (!temp_ref_file.open(QIODevice::WriteOnly | QIODevice::Truncate))
            {
                qWarning() << "Failed to create temporary reference file:" << temp_ref_path;
            }
            temp_ref_file.close();
            QStringList ref_doc = {"DOCX", "ODT", "PPTX"};
            QStringList template_doc = {"PDF", "TEX", "HTML"};
            if (ref_doc.contains(output_extension))
            {
                QProcess ref_proc;
                ref_proc.start("pandoc", {"--print-default-data-file", "reference." + output_extension.toLower()});
                ref_proc.waitForFinished();
                QByteArray ref_data = ref_proc.readAllStandardOutput();
                if (temp_ref_file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                    temp_ref_file.write(ref_data);
                    temp_ref_file.close();
                }
                arguments << "--reference-doc=" + temp_ref_path;
            }
            else if (template_doc.contains(output_extension))
            {
                QString engine = (output_extension == "HTML" ? "html" : "latex");
                QProcess template_proc;
                template_proc.start("pandoc", {"--print-default-template=" + engine});
                template_proc.waitForFinished();
                QByteArray template_data = template_proc.readAllStandardOutput();
                if (temp_ref_file.open(QIODevice::WriteOnly | QIODevice::Truncate))
                {
                    temp_ref_file.write(template_data);
                    temp_ref_file.close();
                }
                arguments << "--template=" + temp_ref_path;
            }
        }
    }
    connect(pandoc, &QProcess::errorOccurred, this, [=](QProcess::ProcessError error)
    {
        Q_UNUSED(error);
        emit conversionFinished(false, "Failed to load document converter.");
        pandoc->deleteLater();
    });
    connect(pandoc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [=](int exitCode, QProcess::ExitStatus status)
    {
        if (!temp_ref_path.isEmpty()) {
            QFile::remove(temp_ref_path);
        }
        else if (status != QProcess::NormalExit || exitCode != 0)
        {
            if (input_path != "") {emit conversionFinished(false, "File could not be converted.");}
            else {emit conversionFinished(false, "No location selected");}
        }
        else
        {
            QFileInfo input_file_info(input_path);
            QString result = "Success: " + input_file_info.completeBaseName() + '.' + input_extension.toLower() + " has been converted to " + input_file_info.completeBaseName() + '.' + output_extension.toLower();
            emit conversionFinished(true, result);
        }
        pandoc->deleteLater();
    });
    pandoc->start("pandoc", arguments);
}

void convert_document_file(QWidget *parent, QString input_extension, QString output_extension, MainDocumentConverter *converter, QString save_folder)
{
    bool is_json = false;
    QString input_info = input_extension + " Files " + "(*." + input_extension.toLower() + ")";
    QString file_path = QFileDialog::getOpenFileName(NULL, "Open File", "", input_info);
    QFileInfo input_file_info(file_path);
    QString output_name = input_file_info.completeBaseName() + "." + output_extension.toLower();
    QString output_path;
    const auto &capabilities = document_capabilities[output_extension.toLower()];
    if (save_folder == "Alternate")
    {
        QString output_info = output_extension + " Files " + "(*." + output_extension.toLower() + ")";
        output_path = QFileDialog::getSaveFileName(NULL, "Save File", "", output_info);
    }
    else
    {
        QDir output_dir(save_folder);
        output_path = output_dir.filePath(output_name);
    }
    if (input_extension == "JSON")
    {
        ifstream json_file((file_path).toStdString());
        json j;
        json_file >> j;
        string markdown = json_markdown_wrap(j);
        QString markdown_file = file_path.left(file_path.length() - 4) + "md";
        save_new_json(markdown_file.toStdString(), markdown);
        format_json(file_path, markdown_file);
        file_path = markdown_file;
        is_json = true;
    }
    QObject::connect(converter, &MainDocumentConverter::conversionFinished, parent, [=](bool success, const QString &message) mutable
    {
        if (success)
        {
            if (is_json)
            {
                QFile md_file(file_path);
                md_file.remove();
            }
        }
        converter->deleteLater();
    });
    converter->convert_document(file_path, output_path, input_extension, output_extension, capabilities);
}
