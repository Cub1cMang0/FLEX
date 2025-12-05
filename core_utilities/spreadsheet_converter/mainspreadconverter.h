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
        void convert_spread(const QString &input_path, const QString &output_path, QString input_extension, QString output_extension) override;
        void convert_xlsx_or_ods(const QString &input_path, const QString &output_path, QString input_extension, QString output_extension);
        void convert_fods_to_csv_or_xml(QString &input_path, QString &output_path, QString input_extension, QString output_extension);
    signals:
        void conversion_finished(bool success, const QString &message);
    private:
        QProcess *python_process = nullptr;
        QString output_path;    
};

void convert_spread_file(QWidget *parent, QString input_extension, QString output_extension, MainSpreadConverter *converter, QString save_folder);
bool check_python_need(QString ext_1, QString ext_2);
bool csv_to_fods(const QString &csv_file, const QString &fods_file, bool &success);
bool fods_to_csv(const QString &fods_file, const QString &csv_file, bool &success);
bool csv_to_xml(const QString &csv_file, const QString &xml_file, bool &success);
bool xml_to_csv(const QString &xml_file, const QString &csv_file, bool &success);
bool fods_to_xml(const QString &fods_file, const QString &xml_file, bool &success);
bool xml_to_fods(const QString &xml_file, const QString &cfods_file, bool &success);
bool find_matching_function(const QString &input_file, const QString &output_file, QString input_extension, QString output_extension, bool &success);

#endif // MAINSPREADCONVERTER_H
