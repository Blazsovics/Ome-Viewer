#include "window3d.h"
#include "ui_window3d.h"

Window3D::Window3D(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Window3D)
{
    ui->setupUi(this);
    ui->screenWidget->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
}

Window3D::~Window3D()
{
    delete ui;
}

void Window3D::setup(TIF_CONTAINER *_tif){
    this->ui->screenWidget->tif = _tif;
}

void Window3D::setColors(unsigned char _colors[9][3]){
    ui->screenWidget->setColors(_colors);
}

void Window3D::setDisplayChannel(int dp){
    this->ui->screenWidget->setDisplayChannel(dp);
}

void Window3D::setLayerDistance(double d)
{
    this->ui->screenWidget->setLayerDistance(d);
}

void Window3D::loadCloud(std::string file)
{
    this->ui->screenWidget->loadCloud(file);
}

void Window3D::toggleLayers()
{
    this->ui->screenWidget->toggleLayers();
}

void Window3D::toggleMarch()
{
    this->ui->screenWidget->toggleMarch();
}

void Window3D::toggleCloud()
{
    this->ui->screenWidget->toggleCloud();
}

void Window3D::setCloudProperties(float _val[2][3])
{
    this->ui->screenWidget->setCloudProperties(_val);
}

void Window3D::calcMarch()
{
    this->ui->screenWidget->calcMarch();
}

void Window3D::ISOchange(int value)
{
    this->ui->screenWidget->ISOchange(value);
}

void Window3D::onFocus()
{
    this->ui->screenWidget->setFocus();
}
