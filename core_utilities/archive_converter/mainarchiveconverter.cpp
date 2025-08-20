#include "mainarchiveconverter.h"
#include <archive.h>
#include <archive_entry.h>
#include <string>
#include <QFileDialog>
#include <QDebug>

MainArchiveConverter::MainArchiveConverter(QObject *parent)
    : QObject(parent)
{}

bool set_output_format(struct archive* a, QString ext) {
    if (ext.toLower() == "zip") return archive_write_set_format_zip(a) == ARCHIVE_OK;
    if (ext.toLower() == "7z") return archive_write_set_format_7zip(a) == ARCHIVE_OK;
    if (ext.toLower() == "tar") return archive_write_set_format_pax_restricted(a) == ARCHIVE_OK;
    if (ext.toLower() == "xar") return archive_write_set_format_xar(a) == ARCHIVE_OK;
    if (ext.toLower() == "iso") return archive_write_set_format_iso9660(a) == ARCHIVE_OK;
    if (ext.toLower() == "cpio") return archive_write_set_format_cpio(a) == ARCHIVE_OK;
    if (ext.toLower() == "mtree") return archive_write_set_format_mtree(a) == ARCHIVE_OK;
    if (ext.toLower() == "warc") return archive_write_set_format_warc(a) == ARCHIVE_OK;
    if (ext.toLower() == "ar") return archive_write_set_format_ar_svr4(a) == ARCHIVE_OK;
    return false;
}

void MainArchiveConverter::convert_archive(const QString &input_path, const QString &output_path, QString input_extension, QString output_extension)
{
    struct archive *input_file = archive_read_new();
    struct archive *output_file = archive_write_new();
    struct archive_entry *entry;
    int r;
    archive_read_support_format_all(input_file);
    archive_read_support_filter_all(input_file);
    if (!set_output_format(output_file, output_extension))
    {
        if (output_extension == "XAR")
        {
            emit conversion_finished(false, "Cannot convert to xar due to OS discrepencies (Non Apple/MacOS)");
        }
        else
        {
            emit conversion_finished(false, "Unsupported output format: " + output_extension);
        }
        return;
    }
    if (archive_write_open_filename(output_file, (output_path.toStdString()).c_str()) != ARCHIVE_OK)
    {
        emit conversion_finished(false, "Failed opening output file: " + output_path);
        return;
    }
    if (archive_read_open_filename(input_file, (input_path.toStdString()).c_str(), 10240) != ARCHIVE_OK)
    {
        emit conversion_finished(false, "Failed opening input file: " + input_path);
        return;
    }
    if (input_path.isEmpty())
    {
        emit conversion_finished(false, "No file selected");
        return;
    }
    while (archive_read_next_header(input_file, &entry) == ARCHIVE_OK)
    {
        if (archive_write_header(output_file, entry) != ARCHIVE_OK) {
            qWarning() << "Write header failed:" << archive_error_string(output_file);
            continue;
        }
        const size_t buf_size = 8192;
        char buf[buf_size];
        ssize_t len;

        while ((len = archive_read_data(input_file, buf, buf_size)) > 0) {
            if (archive_write_data(output_file, buf, len) < 0) {
                qWarning() << "Write data failed:" << archive_error_string(output_file);
                break;
            }
        }
        if (len < 0) {
            qWarning() << "Read data failed:" << archive_error_string(input_file);
        }
    }
    archive_read_close(input_file);
    archive_read_free(input_file);
    archive_write_close(output_file);
    archive_write_free(output_file);
    QFileInfo input_file_info(input_path);
    QString success_message = "Success: " + input_file_info.completeBaseName() + '.' + input_extension.toLower() + " has been converted to " + input_file_info.completeBaseName() + '.' + output_extension.toLower();
    emit conversion_finished(true, success_message);
}

void convert_archive_file(QWidget *parent, QString input_extension, QString output_extension, MainArchiveConverter *converter, QString save_folder)
{
    QString input_info = input_extension + " Files " + "(*." + input_extension.toLower() + ")";
    QString file_path = QFileDialog::getOpenFileName(NULL, "Open File", "", input_info);
    QFileInfo input_file_info(file_path);
    QString output_name = input_file_info.completeBaseName() + "." + output_extension.toLower();
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
    converter->convert_archive(file_path, output_path, input_extension, output_extension);
}
