#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <tifopener.h>
#include <qcustomplot.h>

class Histogram
{
public:
    QCustomPlot* cp;
    QCPItemLine* line_lower;
    QCPItemLine* line_median;
    QCPItemLine* line_upper;
    QSlider* slider_lower;
    QSlider* slider_median;
    QSlider* slider_upper;

    QCPBars* bars;
    TIF_DIR* temp;
    QWidget* mainwindow;
    unsigned median, lower, upper;
    QVector<double> axe_x;
    QVector<double> axe_y;
    Histogram();
    ~Histogram();
    void reset_Y();
    void calc(TIF_DIR* pic);
    void remap(TIF_DIR* pic);
    void save(TIF_DIR* pic);
    void initCustomPlot(QCustomPlot* _cp);
    void refreshCustomPlot();
    void lowerChanged(int, TIF_DIR* pic);
    void medianChanged(int, TIF_DIR* pic);
    void upperChanged(int, TIF_DIR* pic);
    void imageChanged(TIF_DIR* pic);
    void save(int);

};

#endif // HISTOGRAM_H
