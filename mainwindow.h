#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDir>
#include <QColorDialog>
#include <QFileDialog>
#include <iostream>
#include <tifopener.h>
#include <string>
#include <histogram.h>
#include <slideshowwindow.h>
#include <window3d.h>
#include <QOpenGLFunctions_3_3_Core>
#include <cloudopener.h>
#include <glwidget.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    SlideshowWindow slideshow;
    Window3D window3d;
    ~MainWindow();
    TIF_CONTAINER tif_file;
    TIF_CONTAINER tif_backup;
    Histogram histogram;
    unsigned char channel_color[9][3];

    void loadTif(std::string filename);
    void remapCV(int idx = 0);
    void invertImage(int idx = 0);

public slots:
    void slot_empty_temp(){std::cout<<"\nempty temp triggered";};
    void slot_openFileDialogTif();
    void slot_tableWidget_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);

private slots:
    void on_actionOpen_triggered();

    void on_lowerSlider_valueChanged(int value);

    void on_medianSlider_valueChanged(int value);

    void on_upperSlider_valueChanged(int value);

    void on_remapButton_clicked();

    void on_autoRemapButton_clicked();

    void on_slidePopup_clicked();

    void on_threeDPopup_clicked();

    void on_colorButton_clicked();

    void on_spinBox_valueChanged(int arg1);

    void on_doubleSpinBox_valueChanged(double arg1);

    void on_actionOpen_Cloud_triggered();

    void on_toggleLayersButton_clicked();

    void on_toggleMarchButton_clicked();

    void on_toggleCloudButton_clicked();

    void on_cloud_trans_x_valueChanged(double arg1);

    void on_cloud_trans_y_valueChanged(double arg1);

    void on_cloud_trans_z_valueChanged(double arg1);

    void on_cloud_scale_x_valueChanged(double arg1);

    void on_cloud_scale_y_valueChanged(double arg1);

    void on_cloud_scale_z_valueChanged(double arg1);

    void on_revertAllButton_clicked();

    void on_revertButton_clicked();

    void on_cvremapButton_clicked();

    void on_contrastSlider_valueChanged(int value);

    void on_brightnessSlider_valueChanged(int value);

    void on_gammaSlider_valueChanged(int value);

    void on_invertButton_clicked();

    void on_invert_allButton_clicked();

    void on_calcMarchButton_clicked();

    void on_isoSlider_valueChanged(int value);

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
