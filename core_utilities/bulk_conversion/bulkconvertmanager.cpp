#include "bulkconvertmanager.h"
#include <QDebug>
#include <QObject>
#include <QFileInfo>

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

void BulkConvertManager::start()
{
    paused = false;
    cancelled = false;
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

void BulkConvertManager::set_jobs(QSet<QString> input_files, QString output_ext, const QString output_path)
{
    for (const QString &path : input_files)
    {
        ConversionJob job;
        QFileInfo input_info(path);
        job.input_path = path;
        job.output_ext = output_ext;
        job.output_path = output_path;
        job.status = ConversionStatus::Waiting;
        jobs.append(job);
    }
}

void BulkConvertManager::start_av_job(ConversionJob &job, int job_index)
{
    auto *converter = new MainVideoConverter(this);
    connect(converter, &MainVideoConverter::update_result_message, this, [=, &job]
        (bool success, const QString &error)
    {
        if (success)
        {
            job.status = ConversionStatus::Complete;
        }
        else
        {
            job.status = ConversionStatus::Failed;
            job.error_message = error.isEmpty() ? "Conversion failed" : error;
        }
        emit job_updated(job_index);
        current_index++;
        emit progress_updated(current_index, jobs.size());
        converter->deleteLater();
        process_next();
    });
    converter->convert_video(job.input_path, job.output_path, file_ext(job.input_path), job.output_ext);
}

void BulkConvertManager::start_doc_job(ConversionJob &job, int job_index)
{
    auto *converter = new MainDocumentConverter(this);
    connect(converter, &MainDocumentConverter::update_result_message, this, [=, &job]
        (bool success, const QString &error)
    {
        if (success)
        {
            job.status = ConversionStatus::Complete;
        }
        else
        {
            job.status = ConversionStatus::Failed;
            job.error_message = error.isEmpty() ? "Conversion failed" : error;
        }
        emit job_updated(job_index);
        current_index++;
        emit progress_updated(current_index, jobs.size());
        converter->deleteLater();
        process_next();
    });
    converter->convert_document(job.input_path, job.output_path, file_ext(job.input_path), job.output_ext);
}

void BulkConvertManager::process_next()
{
    if (paused || cancelled)
    {
        return;
    }
    if (current_index >= jobs.size())
    {
        emit finished();
        return;
    }
    auto &job = jobs[current_index];
    if (job.status == ConversionStatus::Skipped)
    {
        current_index++;
        process_next();
        return;
    }
    job.status = ConversionStatus::Converting;
    bool success = false;
    QString result_message;
    switch (file_type)
    {
        case FileType::Image:
            result_message = convert_image_file(job.input_path, file_ext(job.input_path), job.output_ext, job.output_path);
            break;
        case FileType::AV:
            start_av_job(job, current_index);
            return;
        case FileType::Doc:
            start_doc_job(job, current_index);
            break;
        case FileType::SS:
            result_message = convert_spread_file(job.input_path, file_ext(job.input_path), job.output_ext, job.output_path);
            break;
        case FileType::Archive:
            result_message = convert_archive_file(job.input_path, file_ext(job.input_path), job.output_ext, job.output_path);
            break;
    }
    if (result_message.left(1) != "S")
    {
        job.error_message = result_message;
    }
    else
    {
        success = true;
    }
    if (success)
    {
        job.status = ConversionStatus::Complete;
    }
    else
    {
        job.status = ConversionStatus::Failed;
        job.error_message = "Conversion failed";
    }
    emit job_updated(current_index);
    current_index++;
    emit progress_updated(current_index, jobs.size());
    process_next();
}
