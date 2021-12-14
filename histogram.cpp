#include "histogram.h"


Histogram::Histogram(){
    slider_lower = nullptr;
    slider_median = nullptr;
    slider_upper = nullptr;
    lower = 0;
    median = 128;
    upper = 255;
    axe_x.resize(256);
    axe_y.resize(256);
    for(unsigned i = 0; i < 256; i++){
        axe_x[i] = (double)i;
        axe_y[i] = 0.0;
    }
    temp = nullptr;
}

Histogram::~Histogram(){
    if(temp != nullptr) delete temp;
}

void Histogram::reset_Y(){
    for(unsigned i = 0; i < 256; i++){
        axe_y[i] = 0;
    }
}

void Histogram::calc(TIF_DIR *pic){
    if(pic == nullptr) return;
    if(pic->data == nullptr) return;
    unsigned dim_u = pic->dim_u;
    unsigned dim_v = pic->dim_v;
    if(temp == nullptr) {
        temp = new TIF_DIR;
        temp->initialize(dim_u, dim_v);
    }
    reset_Y();
    for(unsigned v = 0; v < dim_v; v++){
        for(unsigned u = 0; u < dim_u; u++){
            double value = (double)(pic->index(u, v));
            unsigned axe_idx = unsigned(value);
            if(255 < axe_idx) axe_idx = 255;
            //value /= 255.0;
            //double range = upper - lower;
            //output = output_start + ((output_end - output_start) / (input_end - input_start)) * (input - input_start)
            //value = lower + (range/value);
            //value = value/255.0;
            //temp->index(u, v) = value;
            axe_y[ axe_idx ]++;
        }
    }

    //get to know maximum
    unsigned max_idx = 0;
    double max_occurence = 0;
    for(unsigned i = 0; i < 256; i++){
        if( max_occurence < axe_y[i] ){
            max_occurence = axe_y[i];
            max_idx = i;
        }
    }
    //normalise
    //output = output_start + ( (output_end - output_start) / (input_end - input_start) ) * (input - input_start)
    //axe_y = 0 + ((max_occ - 0) / (1.0-0)) * (axe_y - 0.0) wat.
    //std::cout<<"\nmax_occurence: "<<max_occurence;
    //std::cout<<"\n that is "<<((double)max_occurence/(1024.0 * 1024.0))*100.0<<"% black";
    for(unsigned i = 0; i < 256; i++){
        bool axe_nonzero = axe_y[i] > 0.0;
        //axe_y[i] = ( axe_y[i] / max_occurence); //casting to [0..1]
        if( axe_nonzero )
            axe_y[i] = 10.0 * log10(axe_y[i]/(0.01));
        else {
            axe_y[i] = 0.0;
            continue;
        }
        //axe_y[i] *= 100.0; //display is 128 pixels tall
        if( axe_nonzero ) axe_y[i] += 1.0;
    }
    refreshCustomPlot();
}

void Histogram::remap(TIF_DIR* pic){
    unsigned max_idx = 0; //largest value index, with nonzero occurence
    unsigned min_idx = 255;
    unsigned median_idx = 0;
    for(unsigned a = 255; a > 0;   a--){if( axe_y[a] > 0 ){max_idx = a; break;}}
    for(unsigned a = 0;   a < 256; a++){if( axe_y[a] > 0 ){min_idx = a; break;}}
    if(max_idx == min_idx) return;
    median_idx = (max_idx - min_idx)/2 + min_idx;
    /*
    std::cout<<"\nImage data:"
    <<"\n\tmax_idx: "<<max_idx
    <<"\n\tmin_idx: "<<min_idx
    <<"\n\tmedian_idx: "<<median_idx
    <<"\n\tlower: "<<lower
    <<"\n\tmedian: "<<median
    <<"\n\tupper: "<<upper;
    */

    auto lambda_remap = [](unsigned char value, unsigned start1, unsigned stop1, unsigned start2, unsigned stop2){
        //outgoing = start2 + (stop2 - start2) * ((value - start1) / (stop1 - start1));
        float divisor = (float)(stop1 - start1);
        if(divisor <= 1.0f || divisor != divisor) divisor = 1.0f;
        value = (unsigned char)start2 + (unsigned char)((float)(stop2 - start2) * ((float)(value - start1) / divisor));
        return value;
    };


    unsigned char value = 0;
    for(unsigned v = 0; v < pic->dim_v; v++){
        for(unsigned u = 0; u < pic->dim_u; u++){
            value = pic->index(u,v);
            if( value < median_idx )
                value = lambda_remap(value, min_idx, min_idx + median_idx, lower, median);
            else
                value = lambda_remap(value, min_idx + median_idx, max_idx, median, upper);
            pic->index(u,v) = value;
            //lambda_remap(&pic->index(u,v), min_idx, max_idx, lower, upper);
        }
    }
    this->calc(pic);
    refreshCustomPlot();
}

void Histogram::save(TIF_DIR *pic){
    if(pic->data == nullptr) return;
    if(temp->data == nullptr) return;
    delete [] pic->data;
    pic->data = temp->data;
    temp = nullptr;
}

void Histogram::initCustomPlot(QCustomPlot *_cp){
    cp = _cp;
    cp->setBackground(Qt::darkBlue);
    cp->xAxis->setRange(0.0, 256.0);
    cp->xAxis->setVisible(false);
    cp->yAxis->setRange(0.0, 100.0);
    cp->yAxis->setVisible(false);
    cp->addLayer("lines_layer");
    line_lower = new QCPItemLine(cp);
    line_lower->setLayer("lines_layer");
    line_lower->setPen(QPen(Qt::magenta));
    line_lower->start->setCoords(lower, 0);
    line_lower->end->setCoords(lower, 256);
    line_median = new QCPItemLine(cp);
    line_median->setLayer("lines_layer");
    line_median->setPen(QPen(Qt::yellow));
    line_median->start->setCoords(median, 0);
    line_median->end->setCoords(median, 256);
    line_upper = new QCPItemLine(cp);
    line_upper->setLayer("lines_layer");
    line_upper->setPen(QPen(Qt::red));
    line_upper->start->setCoords(upper, 0);
    line_upper->end->setCoords(upper, 256);
    cp->addLayer("bars_layer", cp->layer("lines_layer"), QCustomPlot::limBelow);
    bars = new QCPBars(cp->xAxis, cp->yAxis);
    bars->setLayer("bars_layer");
    bars->setPen(QPen(Qt::cyan));
    bars->setWidth(1.0);
    bars->setData(axe_x, axe_y);
    cp->replot();
}

void Histogram::refreshCustomPlot(){
    bars->setData(axe_x, axe_y);
    line_lower->start->setCoords(lower, 0);
    line_lower->end->setCoords(lower, 256);
    line_median->start->setCoords(median, 0);
    line_median->end->setCoords(median, 256);
    line_upper->start->setCoords(upper, 0);
    line_upper->end->setCoords(upper, 256);
    cp->replot();
}

void Histogram::lowerChanged(int v, TIF_DIR* pic)
{
    if(median < v) {
        v = median;
        slider_lower->setValue(v);
    }
    this->lower = v;
    pic->lower = v;
    refreshCustomPlot();
}

void Histogram::medianChanged(int v, TIF_DIR* pic)
{
    if(v < lower){
        v = lower;
        slider_median->setValue(v);
    }
    if(upper < v){
        v = upper;
        slider_median->setValue(v);
    }
    median = v;
    pic->median = v;
    refreshCustomPlot();
}

void Histogram::upperChanged(int v, TIF_DIR* pic)
{
    if(v < median) {
        v = median;
        slider_upper->setValue(v);
    }
    this->upper = v;
    pic->upper = v;
    refreshCustomPlot();
}

void Histogram::imageChanged(TIF_DIR* pic)
{
    this->lower = pic->lower;
    this->median = pic->median;
    this->upper = pic->upper;
    this->slider_lower->setValue(lower);
    this->slider_median->setValue(median);
    this->slider_upper->setValue(upper);
    this->calc(pic);
}

void Histogram::save(int image_idx){

}
