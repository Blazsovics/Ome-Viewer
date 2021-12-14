#include "slideshowwindow.h"
#include "ui_slideshowwindow.h"

unsigned SlideshowWindow::helper_get_idx()
{
    unsigned idx = current_time * num_slices + current_slice * num_channels + current_channel;
    if(idx > num_pictures) {std::cout<<"\nindex fault"; return 0;}
    return idx;
}

SlideshowWindow::SlideshowWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SlideshowWindow)
{
    ui->setupUi(this);
    ui_hidden = false;
    current_channel = 0;
    current_slice = 0;
    current_time = 0;
}

SlideshowWindow::~SlideshowWindow()
{
    delete ui;
}

void SlideshowWindow::init(int channels, int slices, int time, unsigned _num_pictures, TIF_CONTAINER* _tif){
    num_pictures = _num_pictures;
    num_slices = slices;
    num_channels = channels;
    num_time = time;
    if(time < 0) ui->time_slider->setDisabled(true);
    else ui->time_slider->setMaximum(time-1);
    if(slices < 0) ui->slice_slider->setDisabled(true);
    else ui->slice_slider->setMaximum(num_slices-1);
    if(channels < 0) ui->slice_slider->setDisabled(true);
    else ui->channel_slider->setMaximum(num_channels-1);
    ui->slice_slider->setMinimum(0);
    ui->channel_slider->setMinimum(0);
    ui->time_slider->setMinimum(0);
    tif = _tif;
}

void SlideshowWindow::on_actionShow_Hide_triggered()
{
    ui_hidden = !ui_hidden;
    if( ui_hidden ){
        ui->channel_slider->hide();
        ui->slice_slider->hide();
        ui->time_slider->hide();
        ui->label->hide();
    } else {
        ui->channel_slider->show();
        ui->slice_slider->show();
        ui->time_slider->show();
        ui->label->show();
    }

}


void SlideshowWindow::on_channel_slider_valueChanged(int value)
{
    current_channel = value;
    unsigned idx = helper_get_idx();
    ui->SlideshowLabel->setPixmap( QPixmap::fromImage(
    tif->directories[idx]->image ) );
}

void SlideshowWindow::on_slice_slider_valueChanged(int value)
{
    current_slice = value;
    unsigned idx = helper_get_idx();
    ui->SlideshowLabel->setPixmap( QPixmap::fromImage(
    tif->directories[idx]->image ) );
}

void SlideshowWindow::on_time_slider_valueChanged(int value)
{
    current_time = value;
    unsigned idx = helper_get_idx();
    ui->SlideshowLabel->setPixmap( QPixmap::fromImage(
    tif->directories[idx]->image ) );
}

