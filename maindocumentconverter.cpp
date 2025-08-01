#include "maindocumentconverter.h"
#include <QString>
#include <QProcess>
#include <QFileDialog>
#include <QDir>
#include <QFile>
#include <QWidget>
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

void MainDocumentConverter::convert_document(const QString &input_path, const QString &output_path, QString input_extension, QString output_extension)
{
    this->output_path = output_path;
    QStringList arguments;
    arguments << input_path << "-o" << output_path;
    pandoc = new QProcess(this);
    connect(pandoc, &QProcess::errorOccurred, this, [=](QProcess::ProcessError error) {
        Q_UNUSED(error);
        emit conversionFinished(false, "Failed to load document converter.");
        pandoc->deleteLater();
    });
    connect(pandoc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [=](int exitCode, QProcess::ExitStatus status) {
                if (status != QProcess::NormalExit || exitCode != 0) {
                    emit conversionFinished(false, "File could not be converted.");
                } else {
                    QFileInfo input_file_info(input_path);
                    QString result = "Success: " + input_file_info.completeBaseName() + '.' + input_extension.toLower() + " has been converted to " + input_file_info.completeBaseName() + '.' + output_extension.toLower();
                    emit conversionFinished(true, result);
                }
                pandoc->deleteLater();
            });
    pandoc->start("pandoc", arguments);
}

void convert_document_file(QWidget *parent, QString input_extension, QString output_extension, MainDocumentConverter *converter)
{
    bool is_json = false;
    QString input_info = input_extension + " Files" + "(*." + input_extension.toLower() + ")";
    QString file_path = QFileDialog::getOpenFileName(NULL, "Open File", "", input_info);
    QDir output_dir("output");
    if (!output_dir.exists())
    {
        output_dir.mkpath(".");
    }
    QFileInfo input_file_info(file_path);
    QString output_name = input_file_info.completeBaseName() + "." + output_extension.toLower();
    QString output_path = output_dir.filePath(output_name);
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
    QObject::connect(converter, &MainDocumentConverter::conversionFinished,
         parent, [=](bool success, const QString &message) mutable {
             if (success) {
                 if (is_json) {
                     QFile md_file(file_path);
                     md_file.remove();
                 }
             }
             converter->deleteLater();
         });
    converter->convert_document(file_path, output_path, input_extension, output_extension);
}
