#include "bulkconvertmanager.h"
#include <QDebug>
#include <QObject>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <QCoreApplication>

BulkConvertManager::BulkConvertManager(QObject *parent)
    : QObject(parent)
{}

QString file_ext(const QString &input_path)
{
    QFileInfo info(input_path);
    QString file_name = info.fileName();
    static const QStringList double_ext =
    {
        "tar.gz", "tar.bz2", "tar.xz", "tar.zst",
        "xar.gz", "xar.bz2", "xar.xz"
    };
    for (const QString &ext : double_ext)
    {
        if (file_name.endsWith("." + ext, Qt::CaseInsensitive))
        {
            return ext;
        }
    }
    return info.suffix();
}

bool correct_ext(FileType file_type, QString ext)
{
    switch (file_type)
    {
        case FileType::Image:
            return (ext == "png" || ext == "jpeg" || ext == "jpg" || ext == "ico" || ext == "jfif" || ext == "pbm" ||
                    ext == "pgm" || ext == "ppm" || ext == "bmp" || ext == "cur" || ext == "xbm" || ext == "xpm");
        case FileType::AV:
            return (ext == "mp4" || ext == "mov" || ext == "avi" || ext == "wmv" || ext == "mkv" || ext == "m4v" ||
                    ext == "mp3" || ext == " wav" || ext == "aiff" || ext == "wma" || ext == "flac" || ext == "alac");
        case FileType::Doc:
            return (ext == "docx" || ext == "epub" || ext == "html" || ext == "json" || ext == "md" || ext == "latex" ||
                    ext == "odt" || ext == "rtf" || ext == "rst" || ext == "org" || ext == "ipynb");
        case FileType::SS:
            return (ext == "xlsx" || ext == "xml" || ext == "csv" || ext == "ods" || ext == "fods");
        case FileType::Archive:
            return (ext == "zip" || ext == "7z" || ext == "tar" || ext == "tar.gz" || ext == "tar.bz2" || ext == "tar.xz" ||
                    ext == "tar.zst" || ext == "xar" || ext == "xar.gz" || ext == "xar.bz2" || ext == "xar.xz" ||
                    ext == "iso" || ext == "cpio" || ext == "ar");
        default:
            return false;
    }
}

void BulkConvertManager::generate_log_name()
{
    QString time_stamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString log_name = "error_log_" + time_stamp + ".txt";
    QString app_dir = QCoreApplication::applicationDirPath();
    QDir error_dir(app_dir + "/error_logs");
    if (!error_dir.exists())
    {
        error_dir.mkpath(".");
    }
    error_log.setFileName(error_dir.absoluteFilePath(log_name));
    error_file_init = true;
}

void BulkConvertManager::start()
{
    paused = false;
    cancelled = false;
    int job_size = jobs.size();
    for (int i = 0; i < job_size; ++i)
    {
        emit job_status_updated(i, ConversionStatus::Waiting);
    }
    error_dir_check();
    process_next();
}

void BulkConvertManager::pause()
{
    if (paused || cancelled)
    {
        return;
    }
    paused = true;
}

void BulkConvertManager::resume()
{
    if (!paused | cancelled)
    {
        return;
    }
    paused = false;
    process_next();
}

void BulkConvertManager::set_file_type(FileType ft)
{
    file_type = ft;
}

void BulkConvertManager::set_jobs(QVector<QString> input_files, QString output_ext, const QString output_path)
{
    for (const QString &path : input_files)
    {
        ConversionJob job;
        QFileInfo input_info(path);
        job.input_path = path;
        job.output_ext = output_ext.toLower();
        job.output_path = output_path;
        job.status = ConversionStatus::Waiting;
        jobs.append(job);
    }
    job_count = jobs.size();
}

void BulkConvertManager::error_dir_check()
{
    QString app_dir = QCoreApplication::applicationDirPath();
    QDir error_dir(app_dir + "/error_logs");
    if (!error_dir.exists())
    {
        error_dir.mkpath(".");
    }
    if (!error_dir.exists())
    {
        error_dir.mkpath(".");
    }
}

bool BulkConvertManager::total_success()
{
    if (succeeded == job_count)
    {
        return true;
    }
    return false;
}

void BulkConvertManager::log_error(QString error)
{
    if (!error_file_init)
    {
        generate_log_name();
    }
    if (error_log.open(QIODevice::Append | QIODevice::Text))
    {
        QTextStream output(&error_log);
        output << error << "\n" << "\n";
        error_log.close();
    }
}

bool BulkConvertManager::log_made()
{
    return (error_file_init);
}

void BulkConvertManager::start_image_job(ConversionJob &job, int job_index)
{
    auto *converter = new MainImageConverter(this);
    connect(converter, &MainImageConverter::update_image_progress, this, [this, job_index] (const QString &error, bool success)
    {
        ConversionJob &job = jobs[job_index];
        if (success)
        {
            job.status = ConversionStatus::Complete;
            emit job_status_updated(job_index, ConversionStatus::Complete);
            succeeded++;
        }
        else
        {
            job.status = ConversionStatus::Failed;
            emit job_status_updated(job_index, ConversionStatus::Failed);
            job.error_message = error.isEmpty() ? "Conversion failed" : error;
            log_error(job.error_message);
        }
        emit job_updated(job_index);
        current_index++;
        emit progress_updated(current_index, job_count);
        sender()->deleteLater();
        process_next();
    });
    converter->convert_image(job.input_path, job.output_path, file_ext(job.input_path), job.output_ext);
}

void BulkConvertManager::start_av_job(ConversionJob &job, int job_index)
{
    auto *converter = new MainVideoConverter(this);
    connect(converter, &MainVideoConverter::update_av_progress, this, [this, job_index] (const QString &error, bool success)
    {
        ConversionJob &job = jobs[job_index];
        if (success)
        {
            job.status = ConversionStatus::Complete;
            emit job_status_updated(job_index, ConversionStatus::Complete);
            succeeded++;
        }
        else
        {
            job.status = ConversionStatus::Failed;
            emit job_status_updated(job_index, ConversionStatus::Failed);
            job.error_message = error.isEmpty() ? "Conversion failed" : error;
            log_error(job.error_message);
        }
        emit job_updated(job_index);
        current_index++;
        emit progress_updated(current_index, job_count);
        sender()->deleteLater();
        process_next();
    });
    converter->convert_video(job.input_path, job.output_path, file_ext(job.input_path), job.output_ext);
}

void BulkConvertManager::start_doc_job(ConversionJob &job, int job_index)
{
    auto *converter = new MainDocumentConverter(this);
    connect(converter, &MainDocumentConverter::update_doc_progress, this, [this, job_index] (const QString &error, bool success)
    {
        ConversionJob &job = jobs[job_index];
        if (success)
        {
            job.status = ConversionStatus::Complete;
            emit job_status_updated(job_index, ConversionStatus::Complete);
            succeeded++;
        }
        else
        {
            job.status = ConversionStatus::Failed;
            emit job_status_updated(job_index, ConversionStatus::Failed);
            job.error_message = error.isEmpty() ? "Conversion failed" : error;
            log_error(job.error_message);
        }
        emit job_updated(job_index);
        current_index++;
        emit progress_updated(current_index, job_count);
        sender()->deleteLater();
        process_next();
    });
    converter->convert_document(job.input_path, job.output_path, file_ext(job.input_path), job.output_ext);
}

void BulkConvertManager::start_ss_job(ConversionJob &job, int job_index)
{
    auto *converter = new MainSpreadConverter(this);
    connect(converter, &MainSpreadConverter::update_ss_progress, this, [this, job_index] (const QString &error, bool success)
    {
        ConversionJob &job = jobs[job_index];
        if (success)
        {
            job.status = ConversionStatus::Complete;
            emit job_status_updated(job_index, ConversionStatus::Complete);
            succeeded++;
        }
        else
        {
            job.status = ConversionStatus::Failed;
            emit job_status_updated(job_index, ConversionStatus::Failed);
            job.error_message = error.isEmpty() ? "Conversion failed" : error;
            log_error(job.error_message);
        }
        emit job_updated(job_index);
        current_index++;
        emit progress_updated(current_index, job_count);
        sender()->deleteLater();
        process_next();
    });
    converter->convert_spread(job.input_path, job.output_path, file_ext(job.input_path), job.output_ext);
}

void BulkConvertManager::start_archive_job(ConversionJob &job, int job_index)
{
    auto *converter = new MainArchiveConverter(this);
    connect(converter, &MainArchiveConverter::update_archive_progress, this, [this, job_index] (const QString &error, bool success)
    {
        ConversionJob &job = jobs[job_index];
        if (success)
        {
            job.status = ConversionStatus::Complete;
            emit job_status_updated(job_index, ConversionStatus::Complete);
            succeeded++;
        }
        else
        {
            job.status = ConversionStatus::Failed;
            emit job_status_updated(job_index, ConversionStatus::Failed);
            job.error_message = error.isEmpty() ? "Conversion failed" : error;
            log_error(job.error_message);
        }
        emit job_updated(job_index);
        current_index++;
        emit progress_updated(current_index, job_count);
        sender()->deleteLater();
        process_next();
    });
    converter->convert_archive(job.input_path, job.output_path, file_ext(job.input_path), job.output_ext);
}

void BulkConvertManager::skip_job(int job_index, ConversionStatus status)
{
    emit job_status_updated(current_index, ConversionStatus::Skipped);
    emit job_updated(current_index);
    current_index++;
    emit progress_updated(current_index, job_count);
    process_next();
}

void BulkConvertManager::process_next()
{
    if (paused || cancelled)
    {
        return;
    }
    if (current_index >= job_count)
    {
        emit finished();
        return;
    }
    auto &job = jobs[current_index];
    job.status = ConversionStatus::Converting;
    emit job_status_updated(current_index, ConversionStatus::Converting);
    switch (file_type)
    {
        case FileType::Image:
        if (correct_ext(file_type, file_ext(job.input_path)) && correct_ext(file_type, job.output_ext))
        {
            start_image_job(job, current_index);
        }
        else
        {
            job.status = ConversionStatus::Skipped;
            skip_job(current_index, ConversionStatus::Skipped);
        }
            break;
        case FileType::AV:
            if (correct_ext(file_type, file_ext(job.input_path)) && correct_ext(file_type, job.output_ext))
            {
                start_av_job(job, current_index);
            }
            else
            {
                job.status = ConversionStatus::Skipped;
                skip_job(current_index, ConversionStatus::Skipped);
            }
            break;
        case FileType::Doc:
            if (correct_ext(file_type, file_ext(job.input_path)) && correct_ext(file_type, job.output_ext))
            {
                start_doc_job(job, current_index);
            }
            else
            {
                job.status = ConversionStatus::Skipped;
                skip_job(current_index, ConversionStatus::Skipped);
            }
            break;
        case FileType::SS:
            if (correct_ext(file_type, file_ext(job.input_path)) && correct_ext(file_type, job.output_ext))
            {
                start_ss_job(job, current_index);
            }
            else
            {
                job.status = ConversionStatus::Skipped;
                skip_job(current_index, ConversionStatus::Skipped);
            }
            break;
        case FileType::Archive:
            if (correct_ext(file_type, file_ext(job.input_path)) && correct_ext(file_type, job.output_ext))
            {
                start_archive_job(job, current_index);
            }
            else
            {
                job.status = ConversionStatus::Skipped;
                skip_job(current_index, ConversionStatus::Skipped);
            }
            break;
    }
}
