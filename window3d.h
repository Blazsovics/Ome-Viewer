#ifndef WINDOW3D_H
#define WINDOW3D_H

#include <QMainWindow>
#include <tifopener.h>
namespace Ui {
class Window3D;
}

class Window3D : public QMainWindow
{
    Q_OBJECT

private:
    Ui::Window3D *ui;

public:
    explicit Window3D(QWidget *parent = nullptr);
    ~Window3D();
    void setup(TIF_CONTAINER* tif);
    void setColors(unsigned char _colors[9][3]);
    void setDisplayChannel(int dp);
    void setLayerDistance(double d);
    void loadCloud(std::string file);
    void toggleLayers();
    void toggleMarch();
    void toggleCloud();
    void setCloudProperties(float _val[2][3]);
    void calcMarch();
    void ISOchange(int value);
public slots:
    void onFocus();

};

#endif // WINDOW3D_H
