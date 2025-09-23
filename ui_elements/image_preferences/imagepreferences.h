#ifndef IMAGEPREFERENCES_H
#define IMAGEPREFERENCES_H

#include <QDialog>

namespace Ui {
class ImagePreferences;
}

class ImagePreferences : public QDialog
{
    Q_OBJECT
    public:
        explicit ImagePreferences(QWidget *parent = nullptr);
        ~ImagePreferences();
    private slots:
        void on_quality_range_textChanged(const QString &numbers);
        void on_quality_cb_stateChanged(int state);
        void on_save_image_preferences_clicked();
        void on_cancel_image_preferences_clicked();
        void fetch_initial_cb_states(QString &aspect_ratio, QPair<Qt::CheckState, QString> &quality, Qt::CheckState &grayscale, Qt::CheckState &alpha, QPair<Qt::CheckState, QString> &bit_depth);
        void load_image_preferences();
        void on_aspect_ratio_currentTextChanged(const QString &arg1);
        void on_quality_cb_checkStateChanged(const Qt::CheckState &arg1);
        void on_quality_range_textEdited(const QString &arg1);
        void on_gray_scale_cb_checkStateChanged(const Qt::CheckState &arg1);
        void on_remove_alpha_cb_checkStateChanged(const Qt::CheckState &arg1);
        void on_bit_depth_cb_checkStateChanged(const Qt::CheckState &arg1);
        void on_bit_depth_currentTextChanged(const QString &arg1);
        void check_checkbox_states();
    private:
        Ui::ImagePreferences *ui;
};

struct ImageFormatCapabilities
{
    bool quality_support = false;
    bool alpha_support = false;
    bool grayscale_support = false;
    bool bit_depth_support = false;
};

inline const QMap<QString, ImageFormatCapabilities> image_capabilities =
    {
        {"png",  {true, true, true, true}},
        {"jpg",  {true, false, true, false}},
        {"jpeg", {true, false, true, false}},
        {"ico",  {false, true, true, true}},
        {"jfif", {true, false, true, false}},
        {"pbm",  {false, false, false, false}},
        {"pgm",  {false, false, true, false}},
        {"ppm",  {false, false, false, false}},
        {"bmp",  {false, false, true, true}},
        {"cur",  {false, true, true, true}},
        {"xbm",  {false, false, false, false}},
        {"xpm",  {false, true, true, false}}
};

#endif // IMAGEPREFERENCES_H
