#ifndef MAINDOCUMENTCONVERTER_H
#define MAINDOCUMENTCONVERTER_H

#include "documentfileconverter.h"
#include <QProcess>
#include <QObject>

class MainDocumentConverter : public QObject, public DocumentFileConverter
{
    Q_OBJECT
    public:
        explicit MainDocumentConverter(QObject *parent = nullptr);
        void convert_document(const QString &input_path, const QString &output_path, QString input_extension, QString output_extension);
    signals:
        void update_doc_progress(const QString &message, bool success);
    private:
        QProcess *pandoc = nullptr;
};

#endif // MAINDOCUMENTCONVERTER_H
