#ifndef BULKCONVERT_H
#define BULKCONVERT_H

#include <QDebug>
#include <QObject>
#include <mainimageconverter.h>
#include <mainvideoconverter.h>
#include <maindocumentconverter.h>
#include <mainspreadconverter.h>
#include <mainarchiveconverter.h>

enum class ConversionStatus
{
    Waiting,
    Converting,
    Skipped,
    Failed,
    Complete
};

enum class FileType
{
    Image,
    AV,
    Doc,
    SS,
    Archive
};

struct ConversionJob
{
    QString input_path;
    QString output_path;
    QString output_ext;
    ConversionStatus status = ConversionStatus::Waiting;
    QString error_message;
};

class BulkConvertManager : public QObject
{
    Q_OBJECT
    public:
        explicit BulkConvertManager(QObject *parent = nullptr);
        void set_file_type(FileType file_type);
        void set_jobs(QVector<QString> input_files, const QString output_ext, const QString output_path);
        void start_image_job(ConversionJob &job, int job_index);
        void start_av_job(ConversionJob &job, int job_index);
        void start_doc_job(ConversionJob &job, int job_index);
        void start_ss_job(ConversionJob &job, int job_index);
        void start_archive_job(ConversionJob &job, int job_index);
        void skip_job(int job_index, ConversionStatus status);
        void start();
        void pause();
        void resume();
        void error_dir_check();
        bool total_success();
        void generate_log_name();
        void log_error(QString error);
        bool log_made();
        bool is_paused() const;
        int succeeded = 0;
        int job_count = 0;

    signals:
        void job_updated(int index);
        void progress_updated(int complete, int total);
        void job_status_updated(int job_index, ConversionStatus status);
        void finished();

    private:
        void process_next();
        QVector<ConversionJob> jobs;
        int current_index = 0;
        bool paused = false;
        bool cancelled = false;
        bool error_file_init = false;
        FileType file_type;
        QFile error_log;

};

QString file_ext(const QString &input_path);
bool correct_ext(FileType file_type, QString ext);

#endif // BULKCONVERT_H
