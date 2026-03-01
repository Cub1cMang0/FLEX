#ifndef MAINARCHIVECONVERTER_H
#define MAINARCHIVECONVERTER_H

#include <QObject>
#include <QFileInfo>
#include <json.hpp>
#include "archivefileconverter.h"

using json = nlohmann::json;

class MainArchiveConverter : public QObject, public ArchiveFileConverter
{
    Q_OBJECT
    public:
        explicit MainArchiveConverter(QObject *parent = nullptr);
        void convert_archive(const QString &input_path, const QString &output_path, QString input_extension, QString output_extension);

    signals:
        void update_archive_progress(const QString &message, bool success);
};

#endif // MAINARCHIVECONVERTER_H
