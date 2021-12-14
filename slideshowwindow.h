#ifndef SLIDESHOWWINDOW_H
#define SLIDESHOWWINDOW_H

#include <QMainWindow>
#include <tifopener.h>

namespace Ui {
class SlideshowWindow;
}

class SlideshowWindow : public QMainWindow
{
    Q_OBJECT
private:
    unsigned helper_get_idx();

public:
    bool ui_hidden;
    TIF_CONTAINER* tif;
    explicit SlideshowWindow(QWidget *parent = nullptr);
    ~SlideshowWindow();
    unsigned num_pictures;
    int num_slices;
    int num_channels;
    int num_time;
    int current_channel;
    int current_slice;
    int current_time;

    void init(int channels, int slices, int time, unsigned _num_pictures, TIF_CONTAINER* _tif);

private slots:
    void on_actionShow_Hide_triggered();

    void on_channel_slider_valueChanged(int value);

    void on_slice_slider_valueChanged(int value);

    void on_time_slider_valueChanged(int value);

private:
    Ui::SlideshowWindow *ui;
};

#endif // SLIDESHOWWINDOW_H
