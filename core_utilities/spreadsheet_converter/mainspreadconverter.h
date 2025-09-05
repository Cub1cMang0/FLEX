#ifndef MAINSPREADCONVERTER_H
#define MAINSPREADCONVERTER_H

#include <QObject>
#include <QProcess>
#include "spreadfileconverter.h"

class MainSpreadConverter : public QObject, public SpreadFileConverter
{
    Q_OBJECT
    public:
        explicit MainSpreadConverter(QObject *parent = nullptr);
        void convert_spread(const QString &input_path, const QString &output_path, QString input_extension, QString output_extension);
    signals:
        void conversion_finished(bool success, const QString &message);
    private:
        QProcess *soffice;
        QString output_path;    
};

void convert_spread_file(QWidget *parent, QString input_extension, QString output_extension, MainSpreadConverter *converter, QString save_folder);

#endif // MAINSPREADCONVERTER_H
