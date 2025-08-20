#ifndef MAINARCHIVECONVERTER_H
#define MAINARCHIVECONVERTER_H

#include <QObject>
#include "archivefileconverter.h"

class MainArchiveConverter : public QObject, public ArchiveFileConverter
{
    Q_OBJECT
    public:
        explicit MainArchiveConverter(QObject *parent = nullptr);
        void convert_archive(const QString &input_path, const QString &output_path, QString input_extension, QString output_extension);
    signals:
        void conversion_finished(bool success, const QString &message);
    private:
        QString output_path;
};

void convert_archive_file(QWidget *parent, QString input_extension, QString output_extension, MainArchiveConverter *converter, QString save_folder);

#endif // MAINARCHIVECONVERTER_H
