#ifndef SPREADFILECONVERTER_H
#define SPREADFILECONVERTER_H

#include <QString>

class SpreadFileConverter
{
    public:
        virtual ~SpreadFileConverter() {}
        void convert_spread(const QString &input_path, const QString &output_path, QString input_extension, QString output_extension);
};

#endif // SPREADFILECONVERTER_H
