#include "mainspreadconverter.h"
#include <QFileDialog>
#include <QDir>
#include <QProcess>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <functional>
#include <string.h>

MainSpreadConverter::MainSpreadConverter(QObject *parent)
    : QObject(parent)
{}

// Uses in order to check if the files that are going to be converted need the python program to convert it

bool check_python_need(QString ext_1, QString ext_2)
{
    if (ext_1 == "xlsx" || ext_2 == "xlsx" || ext_1 == "ods" || ext_2 == "ods")
    {
        return true;
    }
    return false;
}

bool csv_to_fods(const QString &csv_file, const QString &fods_file, bool &success)
{
    success = false;
    QFile csv(csv_file);
    if (!csv.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return false;
    }
    QFile output_file(fods_file);
    if (!output_file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        csv.close();
        return false;
    }
    QXmlStreamWriter w(&output_file);
    w.setAutoFormatting(true);
    w.writeStartDocument();
    w.writeStartElement("office:document");
    w.writeAttribute("xmlns:office", "urn:oasis:names:tc:opendocument:xmlns:office:1.0");
    w.writeAttribute("xmlns:table", "urn:oasis:names:tc:opendocument:xmlns:table:1.0");
    w.writeAttribute("xmlns:text", "urn:oasis:names:tc:opendocument:xmlns:text:1.0");
    w.writeAttribute("xmlns:style", "urn:oasis:names:tc:opendocument:xmlns:style:1.0");
    w.writeAttribute("xmlns:fo", "urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0");
    w.writeAttribute("office:version", "1.3");
    w.writeAttribute("office:mimetype", "application/vnd.oasis.opendocument.spreadsheet");
    if (w.hasError())
    {
        return false;
    }
    w.writeStartElement("office:body");
    w.writeStartElement("office:spreadsheet");
    w.writeStartElement("table:table");
    w.writeAttribute("table:name", "Sheet1");
    if (w.hasError())
    {
        return false;
    }
    QTextStream in(&csv);
    QString first_line = in.readLine();
    csv.seek(0);
    in.seek(0);
    QChar delim = '.';
    if (first_line.count(';') > first_line.count(','))
    {
        delim = '\t';
    }
    else if (first_line.count('\t') > first_line.count(','))
    {
        delim = '\t';
    }
    while (!in.atEnd())
    {
        QString line = in.readLine();
        if (line.isEmpty())
        {
            continue;
        }
        QStringList cells = line.split(',');
        w.writeStartElement("table:table-row");
        for (const QString &c : cells)
        {
            w.writeStartElement("table:table-cell");
            w.writeAttribute("office:value-type", "string");
            w.writeStartElement("text:p");
            w.writeCharacters(c.trimmed());
            w.writeEndElement();
            w.writeEndElement();
            if (w.hasError())
            {
                csv.close();
                output_file.close();
                return false;
            }
        }
        w.writeEndElement();
    }
    w.writeEndElement();
    w.writeEndElement();
    w.writeEndElement();
    w.writeEndElement();
    w.writeEndDocument();
    csv.close();
    output_file.close();
    if (w.hasError())
    {
        return false;
    }
    success = true;
    return true;
}

bool csv_to_xml(const QString &csv_file, const QString &xml_file, bool &success)
{
    success = false;
    QFile csv(csv_file);
    if (!csv.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return false;
    }
    QFile output_file(xml_file);
    if (!output_file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        csv.close();
        return false;
    }
    QXmlStreamWriter writer(&output_file);
    writer.setAutoFormatting(true);
    writer.writeStartDocument();
    writer.writeStartElement("sheet");
    QTextStream stream(&csv);
    while (!stream.atEnd())
    {
        QString line = stream.readLine();
        QStringList cells = line.split(',');
        writer.writeStartElement("row");
        for (const QString &cell : cells)
        {
            writer.writeTextElement("cell", cell.trimmed());
        }
        writer.writeEndElement();
    }
    writer.writeEndElement();
    writer.writeEndDocument();
    csv.close();
    output_file.close();
    success = true;
    return true;
}

bool xml_to_csv(const QString &xml_file, const QString &csv_file, bool &success)
{
    success = false;
    QFile file(xml_file);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Failed to open XML file:" << xml_file;
        return false;
    }

    QByteArray xml_content = file.readAll();
    file.close();
    QFile output_file(csv_file);
    if (!output_file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "Failed to create CSV file:" << csv_file;
        return false;
    }
    QXmlStreamReader reader(xml_content);
    QTextStream out(&output_file);
    std::function<QMap<QString, QString>(QXmlStreamReader&, const QString&, int)> flatten_element;
    flatten_element = [&flatten_element](QXmlStreamReader &r, const QString &parent_key, int depth) -> QMap<QString, QString>
    {
        QMap<QString, QString> result;
        if (depth > 50)
        {
            qDebug() << "Max recursion depth reached";
            return result;
        }
        while (!r.atEnd())
        {
            QXmlStreamReader::TokenType token = r.readNext();
            if (r.hasError())
            {
                qDebug() << "Error during flatten:" << r.errorString();
                return result;
            }
            if (token == QXmlStreamReader::StartElement)
            {
                QString elem_name = r.name().toString();
                QString new_key = parent_key.isEmpty() ? elem_name : parent_key + "_" + elem_name;
                QXmlStreamAttributes attrs = r.attributes();
                for (const auto &attr : attrs)
                {
                    QString attr_key = new_key + "@" + attr.name().toString();
                    result[attr_key] = attr.value().toString();
                }
                QString text = r.readElementText(QXmlStreamReader::IncludeChildElements).trimmed();
                if (!text.isEmpty())
                {
                    QXmlStreamReader temp_reader(QString("<%1>%2</%1>").arg(elem_name).arg(text));
                    temp_reader.readNext();
                    temp_reader.readNext();
                    QString content = temp_reader.readElementText();

                    if (content == text)
                    {
                        result[new_key] = text;
                    }
                    else
                    {
                        QXmlStreamReader nested_reader(QString("<%1>%2</%1>").arg(elem_name).arg(text));
                        nested_reader.readNext();
                        nested_reader.readNext();
                        auto nested_result = flatten_element(nested_reader, new_key, depth + 1);
                        for (auto it = nested_result.begin(); it != nested_result.end(); ++it)
                        {
                            result[it.key()] = it.value();
                        }
                    }
                }
            }
            else if (token == QXmlStreamReader::EndElement)
            {
                break;
            }
        }
        return result;
    };
    QMap<QString, int> element_counts;
    QMap<QString, int> element_depths;
    QMap<QString, bool> has_children;
    QXmlStreamReader counter(xml_content);
    int depth = 0;
    QString last_parent;
    while (!counter.atEnd())
    {
        counter.readNext();
        if (counter.isStartElement())
        {
            QString elem = counter.name().toString();
            element_counts[elem]++;
            element_depths[elem] = depth;

            if (!last_parent.isEmpty())
            {
                has_children[last_parent] = true;
            }

            last_parent = elem;
            depth++;
        }
        else if (counter.isEndElement())
        {
            depth--;
        }
    }
    QString row_element;
    int max_count = 0;
    int best_depth = 999;
    for (auto it = element_counts.begin(); it != element_counts.end(); ++it)
    {
        QString elem = it.key();
        int count = it.value();
        if (count > 1 && has_children.value(elem, false))
        {
            int elem_depth = element_depths[elem];
            if (count > max_count || (count == max_count && elem_depth < best_depth))
            {
                max_count = count;
                row_element = elem;
                best_depth = elem_depth;
            }
        }
    }
    qDebug() << "Detected row element:" << row_element << "with" << max_count << "occurrences at depth" << best_depth;
    QList<QMap<QString, QString>> all_rows;
    QSet<QString> all_headers;
    reader.clear();
    reader.addData(xml_content);
    while (!reader.atEnd())
    {
        reader.readNext();

        if (reader.hasError())
        {
            qDebug() << "XML Parse Error:" << reader.errorString();
            output_file.close();
            return false;
        }
        if (reader.isStartElement() && reader.name().toString() == row_element)
        {
            QMap<QString, QString> row_data = flatten_element(reader, "", 0);
            if (!row_data.isEmpty())
            {
                all_rows << row_data;
                for (const QString &key : row_data.keys())
                {
                    all_headers.insert(key);
                }
            }
        }
    }
    QStringList headers = all_headers.values();
    std::sort(headers.begin(), headers.end());
    if (!headers.isEmpty())
    {
        out << headers.join(',') << "\n";
    }
    for (const auto &row : all_rows)
    {
        QStringList row_values;
        for (const QString &header : headers)
        {
            QString value = row.value(header, "");
            if (value.contains(',') || value.contains('"') || value.contains('\n'))
            {
                value.replace("\"", "\"\"");
                value = "\"" + value + "\"";
            }
            row_values << value;
        }
        out << row_values.join(',') << "\n";
    }
    output_file.close();
    success = (all_rows.size() > 0);
    return success;
}

bool xml_to_fods(const QString &xml_file, const QString &fods_file, bool &success)
{
    QString temp_csv = QDir::temp().filePath("tmp.csv");
    bool xml_success = false;
    if (!xml_to_csv(xml_file, temp_csv, xml_success))
    {
        success = false;
        return false;
    }
    bool fods_success = true;
    if (!csv_to_fods(temp_csv, fods_file, fods_success))
    {
        QFile::remove(temp_csv);
        success = false;
        return false;
    }
    QFile::remove(temp_csv);
    success = true;
    return true;
}

bool find_matching_function(const QString &input_file, const QString &output_file, QString input_extension, QString output_extension, bool &success)
{
    if (input_extension == "csv" && output_extension == "fods") {return csv_to_fods(input_file, output_file, success);}
    else if (input_extension == "xml" && output_extension == "fods") {return xml_to_fods(input_file, output_file, success);}
    else if (input_extension == "csv" && output_extension == "xml") {return csv_to_xml(input_file, output_file, success);}
    else if (input_extension == "xml" && output_extension == "csv") {return xml_to_csv(input_file, output_file, success);}
    success = false;
    return false;
}

void MainSpreadConverter::convert_xlsx_or_ods(const QString &input_path, const QString &output_path, QString input_extension, QString output_extension)
{
    python_process = new QProcess(this);
    QString source_location = QString(__FILE__);
    QFileInfo file_info(source_location);
    QString cpp_directory = file_info.absolutePath();
    QString python_path = cpp_directory + "/python" + "/python.exe";
    QString script;
    if (input_extension == "xlsx" || output_extension == "xlsx") {
        script = cpp_directory + "/python" + "/xlsx_convert.py";
    } else {
        script = cpp_directory + "/python" + "/ods_convert.py";
    }
    QStringList first_arguments;
    first_arguments << script << input_path << output_path;
    connect(python_process, &QProcess::readyReadStandardOutput, this, [=]() {
        QByteArray output = python_process->readAllStandardOutput();
        qDebug() << "Python Output: " << output;
    });
    connect(python_process, &QProcess::readyReadStandardError, this, [=]() {
        QByteArray error = python_process->readAllStandardError();
        qDebug() << "Python Error: " << error;
    });
    connect(python_process, &QProcess::errorOccurred, this, [=](QProcess::ProcessError error) {
        QString error_message = "Failed to load spreadsheet converter.";
        switch (error) {
        case QProcess::FailedToStart:
            error_message = "Failed to start the Python process. Check Python path.";
            break;
        case QProcess::Crashed:
            error_message = "Python process crashed.";
            break;
        case QProcess::Timedout:
            error_message = "Python process timed out.";
            break;
        case QProcess::WriteError:
            error_message = "Error while writing to the Python process.";
            break;
        case QProcess::ReadError:
            error_message = "Error while reading from the Python process.";
            break;
        case QProcess::UnknownError:
            error_message = "Unknown error occurred.";
            break;
        }
        emit conversion_finished(false, error_message);
        python_process->deleteLater();
    });
    connect(python_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [=](int exitCode, QProcess::ExitStatus status) {
        if (status != QProcess::NormalExit || exitCode != 0) {
            QString message = (input_path.isEmpty()) ? "No location selected" : "File could not be converted.";
            emit conversion_finished(false, message);
        } else {
            QFileInfo input_file_info(input_path);
            QString result = "Success: " + input_file_info.completeBaseName() + '.' + input_extension.toLower() +
                             " has been converted to " + input_file_info.completeBaseName() + '.' + output_extension.toLower();
            emit conversion_finished(true, result);
        }
        python_process->deleteLater();
    });
    python_process->start(python_path, first_arguments);
    python_process->waitForFinished(-1);
}

void MainSpreadConverter::convert_fods_to_csv_or_xml(QString &input_path, QString &output_path, QString input_extension, QString output_extension)
{
    python_process = new QProcess(this);
    QString source_location = QString(__FILE__);
    QFileInfo file_info(source_location);
    QString cpp_directory = file_info.absolutePath();
    QString python_path = cpp_directory + "/python" + "/python.exe";
    QString script = cpp_directory + "/python" + "/ods_convert.py";
    QStringList arguments;
    QString output;
    arguments << script << input_path << output_path;
    connect(python_process, &QProcess::readyReadStandardOutput, this, [=]() {
        QByteArray output = python_process->readAllStandardOutput();
        qDebug() << "Python Output: " << output;
    });
    connect(python_process, &QProcess::readyReadStandardError, this, [=]() {
        QByteArray error = python_process->readAllStandardError();
        qDebug() << "Python Error: " << error;
    });
    connect(python_process, &QProcess::errorOccurred, this, [=](QProcess::ProcessError error) {
        QString error_message = "Failed to load spreadsheet converter.";
        switch (error) {
        case QProcess::FailedToStart:
            error_message = "Failed to start the Python process. Check Python path.";
            break;
        case QProcess::Crashed:
            error_message = "Python process crashed.";
            break;
        case QProcess::Timedout:
            error_message = "Python process timed out.";
            break;
        case QProcess::WriteError:
            error_message = "Error while writing to the Python process.";
            break;
        case QProcess::ReadError:
            error_message = "Error while reading from the Python process.";
            break;
        case QProcess::UnknownError:
            error_message = "Unknown error occurred.";
            break;
        }
        emit conversion_finished(false, error_message);
        python_process->deleteLater();
    });
    connect(python_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [=](int exitCode, QProcess::ExitStatus status) {
        if (status != QProcess::NormalExit || exitCode != 0) {
            QString message = (input_path.isEmpty()) ? "No location selected" : "File could not be converted.";
            emit conversion_finished(false, message);
        } else {
            QFileInfo input_file_info(input_path);
            QString result = "Success: " + input_file_info.completeBaseName() + '.' + input_extension.toLower() +
                             " has been converted to " + input_file_info.completeBaseName() + '.' + output_extension.toLower();
            emit conversion_finished(true, result);
        }
        python_process->deleteLater();
    });
    qDebug() << arguments;
    python_process->start(python_path, arguments);
    python_process->waitForFinished(-1);
}

void MainSpreadConverter::convert_spread(const QString &input_path, const QString &output_path, QString input_extension, QString output_extension)
{
    bool success = false;
    bool conversion_result = find_matching_function(input_path, output_path, input_extension, output_extension, success);
    if (conversion_result && success)
    {
        QFileInfo input_file_info(input_path);
        QString result = "Success: " + input_file_info.completeBaseName() + '.' + input_extension.toLower() + " has been converted to " + input_file_info.completeBaseName() + '.' + output_extension.toLower();
        emit conversion_finished(true, result);
    }
    else
    {
        emit conversion_finished(false, "File could not be converted.");
    }
}

void convert_spread_file(QWidget *parent, QString input_extension, QString output_extension, MainSpreadConverter *converter, QString save_folder)
{
    QString input_info = input_extension + " Files " + "(*." + input_extension.toLower() + ")";
    QString file_path = QFileDialog::getOpenFileName(NULL, "Open File", "", input_info);
    QFileInfo input_file_info(file_path);
    QString output_name = input_file_info.completeBaseName() + '.' + output_extension.toLower();
    QString output_path;
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
    if (check_python_need(input_extension.toLower(), output_extension.toLower()))
    {
        converter->convert_xlsx_or_ods(file_path, output_path, input_extension.toLower(), output_extension.toLower());
    }
    else
    {
        if ((input_extension == "FODS" && output_extension == "CSV") || (input_extension == "FODS" && output_extension == "XML"))
        {
            converter->convert_fods_to_csv_or_xml(file_path, output_path, input_extension.toLower(), output_extension.toLower());
        }
        else
        {
            converter->convert_spread(file_path, output_path, input_extension.toLower(), output_extension.toLower());
        }
    }
}
