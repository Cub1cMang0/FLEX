#ifndef DOCUMENTFILECONVERTER_H
#define DOCUMENTFILECONVERTER_H

#include <QString>
#include "documentpreferences.h"
class DocumentFileConverter
{
    public:
        virtual ~DocumentFileConverter() {}
        void convert_document(const QString &input_path, const QString &output_path, QString input_extension, QString output_extension, const DocumentFormatCapabilities &settings);
};

#endif // DOCUMENTFILECONVERTER_H
