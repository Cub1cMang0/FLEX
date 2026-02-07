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
    : DocumentFileConverter(), QObject(parent) {}

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

void MainDocumentConverter::convert_document(const QString &input_path, const QString &output_path, QString input_extension, QString output_extension)
{
    QFileInfo input_file_info(input_path);
    QString output_name = input_file_info.completeBaseName() + "." + output_extension.toLower();
    QString complete_output = QDir(output_path).filePath(output_name);
    const auto &settings = document_capabilities[output_extension.toLower()];
    QString actual_input_path = input_path;
    QString temp_markdown_path;
    bool created_temp_markdown = false;
    if (input_extension.toUpper() == "JSON")
    {
        ifstream json_file(input_path.toStdString());
        if (!json_file.is_open())
        {
            emit update_doc_progress("Failed to open JSON file", false);
            return;
        }
        json j;
        try
        {
            json_file >> j;
            json_file.close();
        }
        catch (const json::exception &e)
        {
            json_file.close();
            emit update_doc_progress(QString("Failed to parse JSON: %1").arg(e.what()), false);
            return;
        }
        string markdown = json_markdown_wrap(j);
        temp_markdown_path = input_path.left(input_path.length() - 4) + "md";
        try
        {
            save_new_json(temp_markdown_path.toStdString(), markdown);
            format_json(input_path, temp_markdown_path);
        }
        catch (...)
        {
            emit update_doc_progress("Failed to create temporary markdown file from JSON", false);
            return;
        }
        actual_input_path = temp_markdown_path;
        created_temp_markdown = true;
    }
    QStringList arguments;
    arguments << actual_input_path << "-o" << complete_output;
    QString temp_ref_path;
    pandoc = new QProcess(this);
    pandoc->setProgram("pandoc");
    QString source_location = QString(__FILE__);
    QFileInfo file_info(source_location);
    QString cpp_directory = file_info.absolutePath();
    QString json_path = cpp_directory + "/conversion_preferences.json";
    ifstream save_json(json_path.toStdString());
    json load_data;
    if (save_json.is_open())
    {
        save_json >> load_data;
        save_json.close();
    }
    if (load_data.contains("document"))
    {
        auto document_preferences = load_data["document"];
        int rem_metadata = document_preferences["rm_metadata"];
        int pres_formatting = document_preferences["pres_formatting"];
        int pres_media = document_preferences["pres_media"];
        int line_break = document_preferences["line_break"];
        int standalone = document_preferences["standalone"];
        if (rem_metadata == 2 && settings.remove_meta_support)
        {
            arguments << "--metadata=title=" << "--metadata=author=" << "--metadata=date=";
        }
        if (pres_media == 2 && settings.preserve_media_support)
        {
            arguments << "--extract-media=media";
        }
        if (line_break == 2 && settings.line_break_support)
        {
            arguments << "--wrap=preserve";
        }
        if (standalone == 2 && settings.standalone_support)
        {
            arguments << "--standalone";
        }
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
            QStringList template_doc = {"TEX", "HTML"};
            if (ref_doc.contains(output_extension))
            {
                QProcess ref_proc;
                ref_proc.start("pandoc", {"--print-default-data-file", "reference." + output_extension.toLower()});
                ref_proc.waitForFinished();
                QByteArray ref_data = ref_proc.readAllStandardOutput();
                if (temp_ref_file.open(QIODevice::WriteOnly | QIODevice::Truncate))
                {
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
        QString result = QString::fromUtf8(pandoc->readAllStandardError());
        if (!temp_ref_path.isEmpty())
        {
            QFile::remove(temp_ref_path);
        }
        if (created_temp_markdown)
        {
            QFile::remove(temp_markdown_path);
        }
        emit update_doc_progress(result, false);
    });
    connect(pandoc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [=](int exitCode, QProcess::ExitStatus status)
    {
        QString result;
        if (!temp_ref_path.isEmpty())
        {
            QFile::remove(temp_ref_path);
        }
        if (created_temp_markdown)
        {
            QFile::remove(temp_markdown_path);
        }
        if (status != QProcess::NormalExit || exitCode != 0)
        {
            result = QString::fromUtf8(pandoc->readAllStandardError());
            emit update_doc_progress(result, false);
        }
        else
        {
            result = QString("Success: %1.%2 has been converted to %3").arg(input_file_info.completeBaseName(),
                input_extension.toLower(), output_name);
            emit update_doc_progress(result, true);
        }
    });
    pandoc->setArguments(arguments);
    pandoc->start();
}
