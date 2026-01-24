#include "videoaudiopreferences.h"
#include "ui_videoaudiopreferences.h"
#include "json.hpp"
#include <fstream>
#include <QFileInfo>

VideoAudioPreferences::VideoAudioPreferences(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::VideoAudioPreferences)
{
    ui->setupUi(this);
    load_videoaudio_preferences();
    ui->save_preferences->setEnabled(false);
    ui->cancel_preferences->setEnabled(false);
    set_tooltips();
}

VideoAudioPreferences::~VideoAudioPreferences()
{
    delete ui;
}

using json = nlohmann::json;
using namespace std;

QString initial_resolution;
QString initial_frame_rate;
QString initial_video_bitrate;
QString initial_video_codec;
QString initial_sampele_rate;
QString initial_channel;
QString initial_audio_bitrate;
QString initial_audio_codec;


void VideoAudioPreferences::fetch_base_preferences()
{
    initial_resolution = ui->video_resolution->currentText();
    initial_frame_rate = ui->frame_rate->currentText();
    initial_video_bitrate = ui->video_bitrate_level->text();
    initial_video_codec = ui->video_codec->currentText();
    initial_sampele_rate = ui->sample_rate->currentText();
    initial_channel = ui->channels->currentText();
    initial_audio_bitrate = ui->audio_bitrate->currentText();
    initial_audio_codec = ui->audio_codec->currentText();

}

void VideoAudioPreferences::set_slider(QString level)
{
    map<string, int> level_map =
    {
        {"N/A", 0},
        {"Low", 1},
        {"Medium", 2},
        {"High", 3},
        {"Very High", 4}
    };
    string level_str = level.toStdString();
    if (level_map.find(level_str) != level_map.end())
    {
        ui->video_br_slider->setValue(level_map[level_str]);
    }
    ui->video_bitrate_level->setText(level);
}

void VideoAudioPreferences::set_tooltips()
{
    QString case_1 = "Applies To Video Files";
    QString case_2 = "Applies To All Files";
    ui->video_resolution->setToolTip(case_1);
    ui->frame_rate->setToolTip(case_1);
    ui->video_br_slider->setToolTip(case_1);
    ui->video_codec->setToolTip(case_1);
    ui->sample_rate->setToolTip(case_2);
    ui->channels->setToolTip(case_2);
    ui->audio_bitrate->setToolTip(case_2);
    ui->audio_codec->setToolTip(case_2);
}

void VideoAudioPreferences::load_videoaudio_preferences()
{
    QString source_location = QString(__FILE__);
    QFileInfo file_info(source_location);
    QString cpp_directory = file_info.absolutePath();
    QString json_path = cpp_directory + "/conversion_preferences.json";
    ifstream save_json(json_path.toStdString());
    if (save_json.is_open())
    {
        json load_data;
        save_json >> load_data;
        if (load_data.contains("video_audio"))
        {
            auto videoaudio_preferences = load_data["video_audio"];
            QString resolution = QString::fromStdString(videoaudio_preferences["resolution"]);
            int resolution_selection = ui->video_resolution->findText(resolution, Qt::MatchFixedString);
            if (resolution_selection >= 0) {ui->video_resolution->setCurrentIndex(resolution_selection);}
            QString frame_rate = QString::fromStdString(videoaudio_preferences["framerate"]);
            int frame_rate_selection = ui->frame_rate->findText(frame_rate, Qt::MatchFixedString);
            if (frame_rate_selection >= 0) {ui->frame_rate->setCurrentIndex(frame_rate_selection);}
            QString video_bitrate = QString::fromStdString(videoaudio_preferences["video_bitrate"]);
            set_slider(video_bitrate);
            QString video_codec = QString::fromStdString(videoaudio_preferences["video_codec"]);
            int video_codec_selection = ui->video_codec->findText(video_codec, Qt::MatchFixedString);
            if (video_codec_selection >= 0) {ui->video_codec->setCurrentIndex(video_codec_selection);}
            QString sample_rate = QString::fromStdString(videoaudio_preferences["sample_rate"]);
            int sample_rate_selection = ui->sample_rate->findText(sample_rate, Qt::MatchFixedString);
            if (sample_rate_selection >= 0) {ui->sample_rate->setCurrentIndex(sample_rate_selection);}
            QString channel = QString::fromStdString(videoaudio_preferences["channel"]);
            int channel_selection = ui->channels->findText(channel, Qt::MatchFixedString);
            if (channel_selection >= 0) {ui->channels->setCurrentIndex(channel_selection);}
            QString audio_bitrate = QString::fromStdString(videoaudio_preferences["audio_bitrate"]);
            int audio_bitrate_selection = ui->audio_bitrate->findText(audio_bitrate, Qt::MatchFixedString);
            if (audio_bitrate_selection >= 0) {ui->audio_bitrate->setCurrentIndex(audio_bitrate_selection);}
            QString audio_codec = QString::fromStdString(videoaudio_preferences["audio_codec"]);
            int audio_codec_selection = ui->audio_codec->findText(audio_codec, Qt::MatchFixedString);
            if (audio_codec_selection >= 0) {ui->audio_codec->setCurrentIndex(audio_codec_selection);}
        }
    }
    fetch_base_preferences();
}

void VideoAudioPreferences::check_boxes_states()
{
    if (initial_resolution == ui->video_resolution->currentText() && initial_frame_rate == ui->frame_rate->currentText() &&
        initial_video_bitrate == ui->video_bitrate_level->text() && initial_video_codec == ui->video_codec->currentText() &&
        initial_sampele_rate == ui->sample_rate->currentText() && initial_channel == ui->channels->currentText() &&
        initial_audio_bitrate == ui->audio_bitrate->currentText() && initial_audio_codec == ui->audio_codec->currentText())
    {
        ui->save_preferences->setEnabled(false);
        ui->cancel_preferences->setEnabled(false);
    }
    else
    {
        ui->save_preferences->setEnabled(true);
        ui->cancel_preferences->setEnabled(true);
    }
}

void VideoAudioPreferences::on_save_preferences_clicked()
{
    QString source_location = QString(__FILE__);
    QFileInfo file_info(source_location);
    QString cpp_directory = file_info.absolutePath();
    QString json_path = cpp_directory + "/conversion_preferences.json";
    json preference_data;
    json videoaudio_data;
    ifstream input_file(json_path.toStdString());
    if (input_file.is_open())
    {
        try
        {
            input_file >> preference_data;
        }
        catch (...)
        {
            json preference_data;
        }
    }
    videoaudio_data["resolution"] = (ui->video_resolution->currentText()).toStdString();
    videoaudio_data["framerate"] = (ui->frame_rate->currentText()).toStdString();
    videoaudio_data["video_bitrate"] = (ui->video_bitrate_level->text()).toStdString();
    videoaudio_data["video_codec"] = (ui->video_codec->currentText()).toStdString();
    videoaudio_data["sample_rate"] = (ui->sample_rate->currentText()).toStdString();
    videoaudio_data["channel"] = (ui->channels->currentText()).toStdString();
    videoaudio_data["audio_bitrate"] = (ui->audio_bitrate->currentText()).toStdString();
    videoaudio_data["audio_codec"] = (ui->audio_codec->currentText()).toStdString();
    preference_data["video_audio"] = videoaudio_data;
    ofstream output_file(json_path.toStdString());
    if (output_file.is_open())
    {
        output_file << preference_data.dump(4);
        output_file.close();
        ui->save_preferences->setEnabled(false);
        ui->cancel_preferences->setEnabled(false);
        fetch_base_preferences();
    }
}

void VideoAudioPreferences::on_cancel_preferences_clicked()
{
    this->close();
}

void VideoAudioPreferences::on_video_resolution_currentTextChanged(const QString &arg1)
{
    check_boxes_states();
}


void VideoAudioPreferences::on_frame_rate_currentTextChanged(const QString &arg1)
{
    check_boxes_states();
}

void VideoAudioPreferences::on_video_br_slider_sliderMoved(int position)
{
    switch (position)
    {
    case 0:
        ui->video_bitrate_level->setText("N/A");
        break;
    case 1:
        ui->video_bitrate_level->setText("Low");
        break;
    case 2:
        ui->video_bitrate_level->setText("Medium");
        break;
    case 3:
        ui->video_bitrate_level->setText("High");
        break;
    case 4:
        ui->video_bitrate_level->setText("Very High");
        break;
    }
    check_boxes_states();
}

void VideoAudioPreferences::on_video_codec_currentTextChanged(const QString &arg1)
{
    check_boxes_states();
}


void VideoAudioPreferences::on_sample_rate_currentTextChanged(const QString &arg1)
{
    check_boxes_states();
}


void VideoAudioPreferences::on_channels_currentTextChanged(const QString &arg1)
{
    check_boxes_states();
}


void VideoAudioPreferences::on_audio_bitrate_currentTextChanged(const QString &arg1)
{
    check_boxes_states();
}


void VideoAudioPreferences::on_audio_codec_currentTextChanged(const QString &arg1)
{
    check_boxes_states();
}
