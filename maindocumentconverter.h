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
        void conversionFinished(bool success, const QString &message);

    private:
        QProcess *pandoc;
        QString output_path;
};

void convert_document_file(QWidget *parent, QString input_extension, QString output_extension, MainDocumentConverter *converter);

#endif // MAINDOCUMENTCONVERTER_H
