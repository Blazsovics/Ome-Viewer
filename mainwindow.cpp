#include "mainwindow.h"
#include "ui_mainwindow.h"

#define ROW ui->tableWidget->currentRow()

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    histogram.slider_lower = ui->lowerSlider;
    histogram.slider_median = ui->medianSlider;
    histogram.slider_upper = ui->upperSlider;
    histogram.initCustomPlot(ui->customPlot);
    ui->Image_Label->setScaledContents(true);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    connect( ui->tableWidget, SIGNAL(currentCellChanged(int,int,int,int)), this, SLOT(slot_tableWidget_currentCellChanged(int,int,int,int)) );
    window3d.setFocusPolicy(Qt::FocusPolicy::ClickFocus);
    //connect(&window3d, SIGNAL(focus()), &window3d, SLOT(onFocus()));
    //connect( this, SIGNAL(signal_menubar_actionOpen()), this, SLOT( slot_openFileDialogTif() ) );
    //connect( ui->lowerSlider, SIGNAL( valueChanged(int) ), histogram, SLOT( slot_lowerChanged(int) ) );
    //window3d(this);
    //window3d.setParent(this);
    //slideshow.setParent(this);
    auto lambda_setColor = [&](unsigned idx, unsigned char r, unsigned char g, unsigned char b){
        channel_color[idx][0] = r;
        channel_color[idx][1] = g;
        channel_color[idx][2] = b;
    };
    lambda_setColor(0, 255, 0, 0);
    lambda_setColor(1, 0, 255, 0);
    lambda_setColor(2, 0, 0, 255);
    lambda_setColor(3, 255, 255, 255);
    lambda_setColor(4, 255, 157, 0);
    lambda_setColor(5, 255, 0, 255);
    lambda_setColor(6, 0, 255, 255);
    lambda_setColor(7, 229, 255, 0);
    lambda_setColor(8, 128, 128, 128);
    /*
    CloudData asd;
    asd.openFile("C:/Users/Bence/Desktop/ome-kepek/storm/bead_testdata_storm.txt");
    */

}

MainWindow::~MainWindow()
{
    ui->tableWidget->setRowCount(0);//proven to delete all items (QTableWidgetItems here Qstrings)
    delete ui;
}

void MainWindow::loadTif(std::string filename){
    ui->filename_label->setText(QString::fromStdString(filename));
    this->tif_file.load(filename);

    auto lambda_set_table_item = [&](int row, int col, QString data){
        QTableWidgetItem *item = new QTableWidgetItem(data);
        ui->tableWidget->setItem(row, col, item );
    };
    ui->tableWidget->setRowCount(tif_file.directories.size());
    for(unsigned i = 0; i < this->tif_file.directories.size(); i++){
        tif_file.directories[i]->refresh_image();
        lambda_set_table_item(i, 0, QString::number(tif_file.directories[i]->dim_u));
        lambda_set_table_item(i, 1, QString::number(tif_file.directories[i]->dim_v));
        //channel
        lambda_set_table_item(i, 2, QString::number(i%3));
        //slice
        lambda_set_table_item(i, 3, QString::number(i%(tif_file.desc_num_slices)));
        //time
        lambda_set_table_item(i, 4, QString::number(tif_file.directories[i]->time_id));
        //texture
        lambda_set_table_item(i, 5, QString::number(i / tif_file.desc_num_channels));
    }
    tif_backup = tif_file;

    slideshow.init( tif_file.desc_num_channels, tif_file.desc_num_slices, tif_file.desc_num_time, tif_file.num_directories, &tif_file );
}

void MainWindow::remapCV(int idx)
{
    if(tif_file.num_directories == 0) return;
    if(tif_file.directories[idx] == nullptr) return;
    if(tif_file.directories[idx]->data == nullptr) return;
    if(tif_backup.num_directories == 0) return;
    if(tif_backup.directories[idx] == nullptr) return;
    if(tif_backup.directories[idx]->data == nullptr) return;
    float alpha = (float)ui->contrastSlider->value()/1000.0f;
    int beta = ui->brightnessSlider->value();
    float gamma = (float)ui->gammaSlider->value()/1000.0f;

    std::cout<<"\nalpha: "<<alpha<<" beta: "<<beta<<" gamma: "<<gamma;
    cv::Mat image = cv::Mat(tif_file.directories[idx]->dim_u,
                            tif_file.directories[idx]->dim_v,
                            CV_8UC1,
                            tif_file.directories[idx]->data);
    cv::Mat backup = cv::Mat(tif_backup.directories[idx]->dim_u,
                             tif_backup.directories[idx]->dim_v,
                             CV_8UC1,
                             tif_backup.directories[idx]->data);
    for( int y = 0; y < image.rows; y++ ) {
        for( int x = 0; x < image.cols; x++ ) {
            image.at<uchar>(y, x) = cv::saturate_cast<uchar>( pow((float)backup.at<uchar>(y,x) / 255.0f, gamma) * 255.0f );
            image.at<uchar>(y, x) = cv::saturate_cast<uchar>( alpha * image.at<uchar>(y,x) + beta );
        }
    }
    tif_file.directories[idx]->refresh_image();
    ui->Image_Label->setPixmap( QPixmap::fromImage(tif_file.directories[idx]->image) );
    histogram.imageChanged(tif_file.directories[idx]);
    this->update();
}

void MainWindow::invertImage(int idx)
{
    if(tif_file.num_directories == 0) return;
    if(tif_file.directories[idx] == nullptr) return;
    if(tif_file.directories[idx]->data == nullptr) return;
    if(tif_backup.num_directories == 0) return;
    if(tif_backup.directories[idx] == nullptr) return;
    if(tif_backup.directories[idx]->data == nullptr) return;
    cv::Mat image = cv::Mat(tif_file.directories[idx]->dim_u,
                            tif_file.directories[idx]->dim_v,
                            CV_8UC1,
                            tif_file.directories[idx]->data);
    for( int y = 0; y < image.rows; y++ ) {
        for( int x = 0; x < image.cols; x++ ) {
            image.at<uchar>(y, x) = cv::saturate_cast<uchar>( image.at<uchar>(y,x) - 255 );
        }
    }
    tif_file.directories[idx]->refresh_image();
    ui->Image_Label->setPixmap( QPixmap::fromImage(tif_file.directories[idx]->image) );
    histogram.imageChanged(tif_file.directories[idx]);
    this->update();

}

void MainWindow::slot_openFileDialogTif()
{
    QString filename = "";
    filename = QFileDialog::getOpenFileName(this, "Open a File", QDir::homePath());
    if(filename == "") return;
    QString extension = filename.mid(filename.length()-4, filename.length()-1);
    std::string str_filename = filename.toStdString();
    std::string str_extension = extension.toStdString();
    if(str_extension == ".tif" || str_extension == "tiff"){
        loadTif(str_filename);
    } else {
        std::cout<<"\nfile: "<<str_filename<<"\n\tis not a valid tif file ( must end in .tif(f) )";
        return;
    }

}


void MainWindow::on_actionOpen_triggered()
{
    this->slot_openFileDialogTif();
}


void MainWindow::slot_tableWidget_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    //std::cout<<"\ntableWidget currentRow: "<<currentRow<<" currentColumn: "<<currentColumn<<" previousRow: "<<previousRow<<" previousColumn "<<previousColumn;
    ui->Image_Label->setPixmap( QPixmap::fromImage(tif_file.directories[currentRow]->image) );
    histogram.imageChanged(tif_file.directories[ROW]);
    this->update();
}



void MainWindow::on_lowerSlider_valueChanged(int value)
{
    histogram.lowerChanged(value, tif_file.directories[ROW]);
    ui->tableWidget->setFocus();
    this->update();
}


void MainWindow::on_medianSlider_valueChanged(int value)
{
    histogram.medianChanged(value, tif_file.directories[ROW]);
    ui->tableWidget->setFocus();
    this->update();
}


void MainWindow::on_upperSlider_valueChanged(int value)
{
    histogram.upperChanged(value, tif_file.directories[ROW]);
    ui->tableWidget->setFocus();
    this->update();
}

void MainWindow::on_remapButton_clicked()
{
    histogram.calc( tif_file.directories[ROW] );
    histogram.remap( tif_file.directories[ROW] );
    ui->tableWidget->setFocus();
    ui->Image_Label->setPixmap( QPixmap::fromImage(tif_file.directories[ROW]->image) );
    histogram.imageChanged(tif_file.directories[ROW]);
    histogram.refreshCustomPlot();
    this->update();
}


void MainWindow::on_autoRemapButton_clicked()
{
    unsigned num_directories = tif_file.directories.size();
    for(unsigned i = 0; i < num_directories; i++){
        std::cout<<"\nremapping layer ["<<i+1<<"/"<<num_directories<<"]..";
        histogram.calc( tif_file.directories[i] );
        histogram.remap( tif_file.directories[i] );
    }
    ui->tableWidget->setFocus();
    ui->Image_Label->setPixmap( QPixmap::fromImage(tif_file.directories[ROW]->image) );
    histogram.imageChanged(tif_file.directories[ROW]);
    histogram.refreshCustomPlot();
    this->update();
}


void MainWindow::on_slidePopup_clicked()
{
    slideshow.show();
}


void MainWindow::on_threeDPopup_clicked()
{
    window3d.setup(&tif_file);
    window3d.setColors(channel_color);
    window3d.show();
}


void MainWindow::on_colorButton_clicked()
{
    /*colors are indexed column-wise!(WEIRD)
     * [0][2][4][...][ ][ ][ ][ ]
     * [1][3][5][   ][ ][ ][ ][ ]
     * i represent it as
     * [c0][c1][c2][...][c7]
     * [background][null][...]
     */
    QColorDialog* dialog = new QColorDialog();
    //set all presets to white
    for(unsigned i = 0; i < 48; i++){
        dialog->setStandardColor(i, Qt::white);
    }
    //set top row to default colrs
    for(unsigned i = 0; i < 8; i++){
        dialog->setStandardColor(i*6, QColor(channel_color[i][0], channel_color[i][1], channel_color[i][2], 255));
    }
    //set 1st of 2nd row to background color
    dialog->setStandardColor(1, QColor(channel_color[8][0], channel_color[8][1], channel_color[8][2], 255));
    //set changeable colors to default in the top row and white under
    for(unsigned i = 0; i < 8; i++){
        dialog->setCustomColor(i*2, QColor(channel_color[i][0], channel_color[i][1], channel_color[i][2], 255));
        dialog->setCustomColor(i*2+1, Qt::white);
    }
    //set 1st of 2nd row to background color
    dialog->setCustomColor(1, QColor(channel_color[8][0], channel_color[8][1], channel_color[8][2], 255));
    //open dialog
    dialog->getColor(Qt::black, this);
    //get colors when dialog is closed
    auto lambda_setColor = [&](unsigned idx, QColor color){
        channel_color[idx][0] = color.red();
        channel_color[idx][1] = color.green();
        channel_color[idx][2] = color.blue();
    };
    //store modified custom colors
    for(unsigned i = 0; i < 8; i++){
        lambda_setColor(i, dialog->customColor(i*2));
    }
    lambda_setColor(8, dialog->customColor(1));
    delete dialog;
    //pass colors to window3d which passes to screenWidget
    window3d.setColors(channel_color);
}


void MainWindow::on_spinBox_valueChanged(int arg1)
{
    window3d.setDisplayChannel(arg1);
}


void MainWindow::on_doubleSpinBox_valueChanged(double arg1)
{
    window3d.setLayerDistance(arg1);
}


void MainWindow::on_actionOpen_Cloud_triggered()
{
    QString filename = "";
    filename = QFileDialog::getOpenFileName(this, "Open a File", QDir::homePath());
    if(filename == "") return;
    QString extension = filename.mid(filename.length()-4, filename.length()-1);
    std::string str_filename = filename.toStdString();
    std::string str_extension = extension.toStdString();
    if(str_extension == ".txt"){
        window3d.loadCloud(str_filename);
    } else {
        std::cout<<"\nfile: "<<str_filename<<"\n\tis not a valid tif file ( must end in .txt )";
        return;
    }
}


void MainWindow::on_toggleLayersButton_clicked()
{
    window3d.toggleLayers();
}


void MainWindow::on_toggleMarchButton_clicked()
{
    window3d.toggleMarch();
}


void MainWindow::on_toggleCloudButton_clicked()
{
    window3d.toggleCloud();
}


void MainWindow::on_cloud_trans_x_valueChanged(double arg1)
{
    float val[2][3] = {
        {(float)ui->cloud_trans_x->value(), (float)ui->cloud_trans_y->value(), (float)ui->cloud_trans_z->value()},
        {(float)ui->cloud_scale_x->value(), (float)ui->cloud_scale_y->value(), (float)ui->cloud_scale_z->value()}
    };
    window3d.setCloudProperties(val);
}
void MainWindow::on_cloud_trans_y_valueChanged(double arg1)
{
    float val[2][3] = {
        {(float)ui->cloud_trans_x->value(), (float)ui->cloud_trans_y->value(), (float)ui->cloud_trans_z->value()},
        {(float)ui->cloud_scale_x->value(), (float)ui->cloud_scale_y->value(), (float)ui->cloud_scale_z->value()}
    };
    window3d.setCloudProperties(val);
}
void MainWindow::on_cloud_trans_z_valueChanged(double arg1)
{
    float val[2][3] = {
        {(float)ui->cloud_trans_x->value(), (float)ui->cloud_trans_y->value(), (float)ui->cloud_trans_z->value()},
        {(float)ui->cloud_scale_x->value(), (float)ui->cloud_scale_y->value(), (float)ui->cloud_scale_z->value()}
    };
    window3d.setCloudProperties(val);
}
void MainWindow::on_cloud_scale_x_valueChanged(double arg1)
{
    float val[2][3] = {
        {(float)ui->cloud_trans_x->value(), (float)ui->cloud_trans_y->value(), (float)ui->cloud_trans_z->value()},
        {(float)ui->cloud_scale_x->value(), (float)ui->cloud_scale_y->value(), (float)ui->cloud_scale_z->value()}
    };
    window3d.setCloudProperties(val);
}
void MainWindow::on_cloud_scale_y_valueChanged(double arg1)
{
    float val[2][3] = {
        {(float)ui->cloud_trans_x->value(), (float)ui->cloud_trans_y->value(), (float)ui->cloud_trans_z->value()},
        {(float)ui->cloud_scale_x->value(), (float)ui->cloud_scale_y->value(), (float)ui->cloud_scale_z->value()}
    };
    window3d.setCloudProperties(val);
}
void MainWindow::on_cloud_scale_z_valueChanged(double arg1)
{
    float val[2][3] = {
        {(float)ui->cloud_trans_x->value(), (float)ui->cloud_trans_y->value(), (float)ui->cloud_trans_z->value()},
        {(float)ui->cloud_scale_x->value(), (float)ui->cloud_scale_y->value(), (float)ui->cloud_scale_z->value()}
    };
    window3d.setCloudProperties(val);
}


void MainWindow::on_revertAllButton_clicked()
{
    tif_file = tif_backup;
    ui->tableWidget->setFocus();
    tif_file.directories[ROW]->refresh_image();
    ui->Image_Label->setPixmap( QPixmap::fromImage(tif_file.directories[ROW]->image) );
    histogram.imageChanged(tif_file.directories[ROW]);
    histogram.refreshCustomPlot();
    this->update();
}


void MainWindow::on_revertButton_clicked()
{
    ui->tableWidget->setFocus();
    (*tif_file.directories[ROW]) = (*tif_backup.directories[ROW]);
    tif_file.directories[ROW]->refresh_image();
    ui->Image_Label->setPixmap( QPixmap::fromImage(tif_file.directories[ROW]->image) );
    histogram.imageChanged(tif_file.directories[ROW]);
    histogram.refreshCustomPlot();
    this->update();
}


void MainWindow::on_cvremapButton_clicked()
{
    for(int i = 0; i < tif_file.num_directories; i++) remapCV(i);
}


void MainWindow::on_contrastSlider_valueChanged(int value)
{
    remapCV(ROW);
}


void MainWindow::on_brightnessSlider_valueChanged(int value)
{
    remapCV(ROW);
}


void MainWindow::on_gammaSlider_valueChanged(int value)
{
    remapCV(ROW);
}




void MainWindow::on_invertButton_clicked()
{
    invertImage(ROW);
}


void MainWindow::on_invert_allButton_clicked()
{
    for(unsigned i = 0; i < tif_file.num_directories; i++){
        invertImage(i);
    }
}


void MainWindow::on_calcMarchButton_clicked()
{
    window3d.calcMarch();
}


void MainWindow::on_isoSlider_valueChanged(int value)
{
    window3d.ISOchange(value);
}

