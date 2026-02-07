#ifndef MAINSPREADCONVERTER_H
#define MAINSPREADCONVERTER_H

#include <QObject>
#include <QProcess>
#include "spreadfileconverter.h"
#include "json.hpp"

using json = nlohmann::json;

class MainSpreadConverter : public QObject, public SpreadFileConverter
{
    Q_OBJECT
    public:
        explicit MainSpreadConverter(QObject *parent = nullptr);
        void convert_spread(const QString &input_path, const QString &output_path, QString input_extension, QString output_extension);
        void convert_xlsx_or_ods(const QString &input_path, const QString &output_path, QString input_extension, QString output_extension);
        void convert_fods_to_csv_or_xml(QString &input_path, QString &output_path, QString input_extension, QString output_extension);
        void convert_spread_file(QString file_path, QString input_extension, QString output_extension, QString save_folder);
        void csv_to_fods(const QString &csv_file, const QString &fods_file, json pref_file);
        void csv_to_xml(const QString &csv_file, const QString &xml_file, json pref_file);
        void xml_to_csv(const QString &xml_file, const QString &csv_file, json pref_file);
        void xml_to_fods(const QString &xml_file, const QString &cfods_file, json pref_file);
        void find_matching_function(const QString &input_file, const QString &output_file, QString input_extension, QString output_extension);
    signals:
        void update_ss_progress(const QString &message, bool success);
    private:
        QProcess *python_process;
};

bool check_python_need(QString ext_1, QString ext_2);

#endif // MAINSPREADCONVERTER_H
