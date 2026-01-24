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
    void set_jobs(QSet<QString> input_files, const QString output_ext, const QString output_path);
    void start_av_job(ConversionJob &job, int job_index);
    void start_doc_job(ConversionJob &job, int job_index);
    void start();
    void pause();
    void resume();
    void cancel();
    bool is_paused() const;
signals:
    void job_updated(int index);
    void progress_updated(int complete, int total);
    void finished();
private:
    void process_next();
    QVector<ConversionJob> jobs;
    int current_index = 0;
    bool paused = false;
    bool cancelled = false;
    FileType file_type;
};

QString file_ext(const QString &input_path);

#endif // BULKCONVERT_H
