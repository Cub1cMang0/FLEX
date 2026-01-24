#ifndef MAINARCHIVECONVERTER_H
#define MAINARCHIVECONVERTER_H

#include <QObject>
#include "archivefileconverter.h"

class MainArchiveConverter : public QObject, public ArchiveFileConverter
{
    Q_OBJECT
    public:
        explicit MainArchiveConverter(QObject *parent = nullptr);
        bool convert_archive(const QString &input_path, const QString &output_path, QString input_extension, QString output_extension);
    signals:
        void conversion_finished(bool success, const QString &message);
};

QString convert_archive_file(QString file_path, QString input_extension, QString output_extension, QString save_folder);

#endif // MAINARCHIVECONVERTER_H
