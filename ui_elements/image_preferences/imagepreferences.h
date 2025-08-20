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

inline const QMap<QString, ImageFormatCapabilities> format_capabilities =
    {
        {"png",  {true, true, true, false}},
        {"jpg",  {true, false, true, false}},
        {"jpeg", {true, false, true, false}},
        {"ico",  {false, true, true, false}},
        {"jfif", {true, false, true, false}},
        {"pbm",  {false, false, true, true}},
        {"pgm",  {false, false, true, true}},
        {"ppm",  {false, false, true, true}},
        {"bmp",  {false, false, true, false}},
        {"cur",  {false, true, true, false}},
        {"xbm",  {false, false, true, true}},
        {"xpm",  {false, true, true, true}}
};

#endif // IMAGEPREFERENCES_H
