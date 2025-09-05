#ifndef VIDEOAUDIOPREFERENCES_H
#define VIDEOAUDIOPREFERENCES_H

#include <QDialog>

namespace Ui {
class VideoAudioPreferences;
}

class VideoAudioPreferences : public QDialog
{
    Q_OBJECT

public:
    explicit VideoAudioPreferences(QWidget *parent = nullptr);
    ~VideoAudioPreferences();

private slots:
    void load_videoaudio_preferences();

    void check_boxes_states();

    void fetch_base_preferences();

    void set_slider(QString level);

    void on_video_br_slider_sliderMoved(int position);

    void on_cancel_preferences_clicked();

    void on_save_preferences_clicked();

    void set_tooltips();

    void on_video_resolution_currentTextChanged(const QString &arg1);

    void on_frame_rate_currentTextChanged(const QString &arg1);

    void on_video_codec_currentTextChanged(const QString &arg1);

    void on_sample_rate_currentTextChanged(const QString &arg1);

    void on_channels_currentTextChanged(const QString &arg1);

    void on_audio_bitrate_currentTextChanged(const QString &arg1);

    void on_audio_codec_currentTextChanged(const QString &arg1);

private:
    Ui::VideoAudioPreferences *ui;
};

#endif // VIDEOAUDIOPREFERENCES_H
