#ifndef ARCHIVEFILECONVERTER_H
#define ARCHIVEFILECONVERTER_H

#include <QString>
class ArchiveFileConverter
{
    public:
        virtual ~ArchiveFileConverter() {}
        bool convert_archive(const QString &input_path, const QString &output_path, QString input_extension, QString output_extension);
};

#endif // ARCHIVEFILECONVERTER_H
