#ifndef TIFOPENER_H
#define TIFOPENER_H

#include <iostream>
#include <vector>
#include <QString>
#include <QPixmap>
#include <QImage>
#include <libtiff/tiff.h>
#include <libtiff/tiffio.h>
//ˇˇfor image description map
#include <map>
#include <sstream>
#include <cmath>


struct TIF_DIR{
    //this container only has one image layer.. a byte array
public:
    QImage image;
    unsigned char* data;
    unsigned dim_u;
    unsigned dim_v;
    unsigned bits_per_sample;//bits per sample
    unsigned texture_id;//associated texture where this will be a channel
    unsigned channel_id;
    unsigned slice_id;
    unsigned time_id;
    unsigned lower;//histogram slider values
    unsigned median;
    unsigned upper;

    void clear();
    TIF_DIR();
    ~TIF_DIR();
    unsigned char& index(unsigned u, unsigned v);
    void initialize(unsigned _dim_u, unsigned _dim_v);
    void refresh_image();
    void operator=(const TIF_DIR& other);

};

inline void TIF_DIR::clear()
{
    if(data != nullptr) delete [] data;
}

inline TIF_DIR::TIF_DIR(){
    data = nullptr;
    dim_u = 0;
    dim_v = 0;
    bits_per_sample = 0;
    texture_id = 0;
    channel_id = 0;
    slice_id = 0;
    time_id = 0;
    lower = 0;
    median = 128;
    upper = 255;
}

inline TIF_DIR::~TIF_DIR()
{
    this->clear();
}


inline unsigned char &TIF_DIR::index(unsigned u, unsigned v){
    if(u < dim_u && v < dim_v){
        return data[v * dim_u + u];
    }
    else return data[0];
}

inline void TIF_DIR::initialize(unsigned _dim_u, unsigned _dim_v)
{
    if(data != nullptr) delete [] data;
    dim_u = _dim_u;
    dim_v = _dim_v;
    data = new unsigned char [ dim_u * dim_v ];
}

inline void TIF_DIR::refresh_image()
{
    if(data == nullptr) return;
    else {
        image = QImage(data, dim_u, dim_v, dim_u, QImage::Format_Grayscale8);
    }
}

inline void TIF_DIR::operator=(const TIF_DIR &other){
    this->dim_u  = other.dim_u;
    this->dim_v  = other.dim_v;
    this->bits_per_sample  = other.bits_per_sample;
    this->texture_id  = other.texture_id;
    this->channel_id  = other.channel_id;
    this->slice_id  = other.slice_id;
    this->time_id  = other.time_id;
    this->lower  = other.lower;
    this->median  = other.median;
    this->upper  = other.upper;
    if(other.data != nullptr){
        std::copy(other.data, other.data+(dim_u*dim_v), this->data);
    } else this->data = nullptr;
    this->image = other.image;
};

struct ModCounter {
    unsigned max_value;
    unsigned current_value;
    ModCounter* next_num;
    ModCounter(){max_value = 0;current_value = 0;next_num=nullptr;}
    ModCounter(unsigned _max_value, ModCounter* _next_num = nullptr): max_value(_max_value), next_num(_next_num){current_value = 0;}
    void add(){
        if(this->max_value == 0) return;
        this->current_value++;
        if((this->current_value % this->max_value) == 0){
            if(next_num != nullptr) next_num->add();
            this->current_value = 0;
        }
    }
};

struct TIF_CONTAINER {
    //a 1 byte per pixel representation per channel
    std::vector<TIF_DIR*> directories;
    char* image_desc_ptr = nullptr;
    unsigned num_directories = 0;
    unsigned width = 0;
    unsigned height = 0;
    unsigned n_pixels = 0;
    unsigned bits_per_sample = 0;
    std::string filename;
    //^^"bits per component" could be different per channel
    //but also "LibTiff does not support different BitsPerSample values for different components."
    unsigned samples_per_pixel = 0;
    //^^"components per pixel" = num_channels
    #define num_channels samples_per_pixel
    //values from file descriptionˇˇ
    int desc_num_channels = 0; //dim C
    int desc_num_slices = 0;   //dim Z
    int desc_num_time = 0;     //dim T
    bool no_order = false;
    std::map<std::string, int> description_map;
    std::map<std::string, float> floats_map;
    size_t channel = 0;
    size_t slice = 0;
    size_t time = 0;
    std::stringstream ss;
    ModCounter channel_cnt;
    ModCounter slice_cnt;
    ModCounter time_cnt;
    ModCounter* quickest;
    bool CZT = false; //found the exact string DimensionOrder="XYCZT" in description
private:
void inline getProperties();
void inline seekAndInsert(std::string source, std::string var_name, std::string key_str, size_t& location);
void inline seekAndInsertFloat(std::string source, std::string var_name, std::string key_str);

void inline getDescription();
void inline setCounters();

void inline sortDirectories();
void inline loadData();
public:
    TIF_CONTAINER();
    ~TIF_CONTAINER();
    void operator=(const TIF_CONTAINER &other);
    void load(std::string filename);
    void clear();

};

inline void TIF_CONTAINER::getProperties(){//block so some local variables not needed anymore expire
    /* count directories*/
    TIFF* dir_counter_instance = nullptr;
    dir_counter_instance = TIFFOpen((filename).c_str(), "r");
    if( dir_counter_instance == nullptr ) return; //if opening the file fails, break lambda function
    else {
        std::cout<<"\nloaded image: "<<filename;
        image_desc_ptr = nullptr;
        TIFFGetField(dir_counter_instance, TIFFTAG_IMAGEDESCRIPTION, &image_desc_ptr);
        TIFFGetField(dir_counter_instance, TIFFTAG_IMAGEWIDTH, &width);
        TIFFGetField(dir_counter_instance, TIFFTAG_IMAGELENGTH, &height);
        TIFFGetField(dir_counter_instance, TIFFTAG_BITSPERSAMPLE, &bits_per_sample);
        TIFFGetField(dir_counter_instance, TIFFTAG_SAMPLESPERPIXEL, &samples_per_pixel);
        n_pixels = width * height;
        do {
            num_directories++;
        } while (TIFFReadDirectory(dir_counter_instance));
        TIFFClose(dir_counter_instance);
        std::cout<<"\nproperties tab";
        std::cout<<"\n\tnum_directories: "<<num_directories
                <<"\n\timage width: "<<width
               <<"\n\theight: "<<height
              <<"\n\tbits_per_sample: "<<bits_per_sample
             <<"\n\tsamples_per_pixel: "<<samples_per_pixel<<"\n";
    }
}

inline void TIF_CONTAINER::seekAndInsert(std::string source, std::string var_name, std::string key_str, size_t &location){
    //seek location of string, and insert to description map with value. insert 0 if nothing found
    int value = -1;
    if(description_map.find(var_name) == description_map.end()){
        //if entry does not yet exist, add as not found
        description_map.insert( std::pair<std::string, int>(var_name, value) );
    } else {
        //if entry exist other than not found, return
        if(description_map[var_name] > value) return;
    }
    size_t first = source.find(key_str);
    if(first == std::string::npos) return;
    //if key is not found, return
    //else ~something is found
    location = first;
    source = source.substr(first + key_str.size()-1, 5);
    //find end of word
    size_t first_digit = 0;
    size_t last_digit = 0;
    std::string number = "";
    for(int i = 0; i < 6; i++){
        if(isdigit(source[i])) {first_digit = i; break;}
    }
    for(int i = 5; 0 <= i; i--){
        if(isdigit(source[i])) {last_digit = i; break;}
    }

    if(first_digit > last_digit) return;
    number = source.substr(first_digit, last_digit - first_digit + 1);
    if((number.length() == 1) && !isdigit(number[0])){return;}
    ss.clear(); ss.str("");
    ss<<number; ss>>value;
    description_map[var_name] = value;
}

inline void TIF_CONTAINER::seekAndInsertFloat(std::string source, std::string var_name, std::string key_str){
    /* check map for var name, add as not found */
    float value = -1.0f;
    if(floats_map.find(var_name) == floats_map.end()){
        //if entry does not yet exist, add as not found
        floats_map.insert( std::pair<std::string, float>(var_name, value) );
    } else {
        //if entry exist other than not found, return
        if(floats_map[var_name] > 0.0f) return;
    }
    /*look for key*/
    size_t s[3];
    s[0] = source.find(key_str);
    s[1] = source.find('"', s[0]+1);
    s[2] = source.find('"', s[1]+1);
    if(s[0] == std::string::npos ||
            s[1] == std::string::npos ||
            s[2] == std::string::npos) {return;}
    std::string number = source.substr(s[1]+1, s[2]-s[1]-1);
    /* insert in map */
    ss.clear(); ss.str("");
    ss<<number; ss>>value;
    floats_map[var_name] = value;
    std::cout<<" value "<<value;
}

inline void TIF_CONTAINER::getDescription(){
    /* read proper data from description */
    std::string description;
    if(image_desc_ptr == nullptr){
        description = "";
        std::cout<<"\nno description found";
        no_order = true;
        desc_num_channels   = -1;
        desc_num_slices     = -1;
        desc_num_time       = -1;
        return;
    } else {
        description = std::string(image_desc_ptr);
        //delete [] image_desc_ptr;
    }
    /*DimensionOrder="XYCZT" ID="Pixels:0" PhysicalSizeX="0.08056" PhysicalSizeY="0.08056" PhysicalSizeZ="0.15" SizeC="1" SizeT="1" SizeX="512" SizeY="512" SizeZ="1"*/
    if(description.find("DimensionOrder=\"XYCZT\"") != std::string::npos) {CZT = true;}
    seekAndInsert(description, "channels", "channels", channel);
    seekAndInsert(description, "slices", "slices", slice);
    seekAndInsert(description, "time", "time", time);
    seekAndInsert(description, "channels", " C", channel);
    seekAndInsert(description, "slices", " S", slice);
    seekAndInsert(description, "time", " T", time);
    seekAndInsert(description, "channels", " SizeC", channel);
    seekAndInsert(description, "slices", " SizeZ", slice);
    seekAndInsert(description, "time", " SizeT", time);
    //lf floats
    seekAndInsertFloat(description, "fsize_x", "PhysicalSizeX");
    seekAndInsertFloat(description, "fsize_y", "PhysicalSizeY");
    seekAndInsertFloat(description, "fsize_z", "PhysicalSizeZ");
    seekAndInsertFloat(description, "ftime", " TimeIncrement");
    std::cout<<"\ndescription tab";
    std::cout<<"\n\tchannels:\t"<<description_map["channels"];
    std::cout<<"\n\tslices:\t"<<description_map["slices"];
    std::cout<<"\n\ttime:\t"<<description_map["time"];
    std::cout<<"\n\tfsize_x:\t"<<floats_map["fsize_x"];
    std::cout<<"\n\tfsize_y:\t"<<floats_map["fsize_y"];
    std::cout<<"\n\tfsize_z:\t"<<floats_map["fsize_z"];
    std::cout<<"\n\tftime:\t"<<  floats_map["ftime"];
    std::cout<<"\n\tdescription {\n";
    std::cout<<description<<"\n}\n";
    desc_num_channels = description_map["channels"];
    desc_num_slices = description_map["slices"];
    desc_num_time = description_map["time"];
    if(
            desc_num_channels   <= 1 &&
            desc_num_slices     <= 1 &&
            desc_num_time       <= 1
            ) no_order = true;
}



inline void TIF_CONTAINER::setCounters(){
    channel_cnt.max_value = desc_num_channels;
    slice_cnt.max_value = desc_num_slices;
    time_cnt.max_value = desc_num_time;
    quickest = nullptr;
    if(CZT){
        quickest = &channel_cnt;
        channel_cnt.next_num = &slice_cnt;
        slice_cnt.next_num = &time_cnt;
        return;
    }
    if(channel < slice && channel < time){
        quickest = &channel_cnt;
        if(slice < time){
            channel_cnt.next_num = &slice_cnt;
            slice_cnt.next_num = &time_cnt;
        } else {
            channel_cnt.next_num = &time_cnt;
            time_cnt.next_num = &slice_cnt;
        }
    } else if(slice < channel && slice < time){
        quickest = &slice_cnt;
        if(channel < time){
            slice_cnt.next_num = &channel_cnt;
            channel_cnt.next_num = &time_cnt;
        } else {
            slice_cnt.next_num = &time_cnt;
            time_cnt.next_num = &channel_cnt;
        }
    } else if(time < channel && time < slice){
        quickest = &time_cnt;
        if(channel < slice){
            time_cnt.next_num = &channel_cnt;
            channel_cnt.next_num = &slice_cnt;
        } else {
            time_cnt.next_num = &slice_cnt;
            slice_cnt.next_num = &channel_cnt;
        }
    }
}

inline void TIF_CONTAINER::sortDirectories(){
    //sort the vector to CZT format where channel changes the quickest
    if(CZT) return;
    if(!no_order){
        std::sort(directories.begin(), directories.end(), [](const TIF_DIR* a, const TIF_DIR* b) -> bool
        {return (a->time_id * 1024 + a->slice_id * 32 + a->channel_id) <
                    (b->time_id * 1024 + b->slice_id * 32 + b->channel_id);}
        );
    }
}

inline void TIF_CONTAINER::loadData(){
    //separate directories based on channels, slices and time
    //  >each TIF_DIR structure will contain 1 channel (usually 16b/pixel)
    //  >that one channel is scaled to 1 byte
    //  >this method is also ready to handle R***8888 type directories
    //  (R***8888 == RGBA8888 || R***8888 || RG**8888 || RGB*8888)
    //  >this method is also ready to handle single channel types with any bits_per_sample between 1..32

    TIFF* tif = nullptr;
    tif = TIFFOpen((filename).c_str(), "r");
    if( tif == nullptr ) {std::cout<<"\nsecond open fail in load - this should never happen\n"; return;}

    auto lambda_tiff_setIntRaster = [&](uint32_t* _raster){
        //read the integer raster and switch directory
        TIFFReadRGBAImage(tif, width, height, _raster, 0);
        TIFFReadDirectory(tif);
    };

    /*integer parser B32 - theese variables are on the same memory*/
    union b32 {
        unsigned char c[4];
        uint32_t i;
    };
    b32 B32;
    b32 Conversion;
    double maximum_pixel_value = std::pow(2, bits_per_sample)-1;

    auto lambda_getIntRaster = [&](uint32_t*& _raster, TIF_DIR*& _pic, unsigned _u, unsigned _v){
        //<TIF_DIR>pic usually only contains one channel
        B32.i = _raster[_v * width + _u];
        for(unsigned i = 0; i < 4; i++) Conversion.c[i] = B32.c[3-i]; //convert byte order 0123 to 3210
        //rescale to byte per pixel - usually the stored format is 16b/pixel grayscale
        _pic->index(_u,_v) = (unsigned char)( ((double)Conversion.i/maximum_pixel_value) * 255.0 );
    };
    auto lambda_getIntRasterChannel = [&](uint32_t*& _raster, TIF_DIR*& _pic, unsigned channel, unsigned _u, unsigned _v){
        B32.i = _raster[_v * width + _u];
        _pic->index(_u,_v) = (unsigned char)B32.c[channel];
    };

    auto lambda_Count = [&](TIF_DIR*& _pic){
        if(no_order){
            _pic->channel_id = 0;
            _pic->slice_id = 0;
            _pic->time_id = 0;
        } else {
            _pic->channel_id = channel_cnt.current_value;
            _pic->slice_id = slice_cnt.current_value;
            _pic->time_id = time_cnt.current_value;
            quickest->add();
        }
    };

    uint32_t* raster = (uint32_t*)_TIFFmalloc(n_pixels * 4);

    for(unsigned i = 0; i < num_directories; i++){
        std::cout<<"\nloading image "<<i<<"...";
        lambda_tiff_setIntRaster(raster);
        unsigned idx = directories.size();
        for(unsigned c = 0; c < num_channels; c++){
            //this is for if a directory has channels within it like RGBA
            directories.push_back(new TIF_DIR);
            directories[idx + c]->initialize(width, height);
            directories[idx + c]->bits_per_sample = bits_per_sample;
            lambda_Count(directories[idx + c]);
        }

        for(unsigned v = 0; v < height; v++){
            for(unsigned u = 0; u < width; u++){
                if(num_channels == 1){
                    lambda_getIntRaster(raster, directories[idx], u ,v);
                    continue;
                } else {
                    for(unsigned c = 0; c < num_channels; c++){
                        lambda_getIntRasterChannel(raster, directories[idx+c], c, u ,v);
                    }
                }
            }
        }

    }

    _TIFFfree(raster);
    TIFFClose(tif);
}

inline TIF_CONTAINER::TIF_CONTAINER(){
    description_map["channels"] =   -1;
    description_map["slices"] =     -1;
    description_map["time"] =       -1;
    floats_map["fsize_x"] =         -1.0f;
    floats_map["fsize_y"] =         -1.0f;
    floats_map["fsize_z"] =         -1.0f;
    floats_map["ftime"] =           -1.0f;
    /*
    std::cout<<"\ninit tab";
    std::cout<<"\n\tchannels:\t"<<description_map["channels"];
    std::cout<<"\n\tslices:\t"<<  description_map["slices"];
    std::cout<<"\n\ttime:\t"<<    description_map["time"];
    std::cout<<"\n\tfsize_x:\t"<< floats_map["fsize_x"];
    std::cout<<"\n\tfsize_y:\t"<< floats_map["fsize_y"];
    std::cout<<"\n\tfsize_z:\t"<< floats_map["fsize_z"];
    std::cout<<"\n\tftime:\t"<<   floats_map["ftime"];
    */
}

inline TIF_CONTAINER::~TIF_CONTAINER(){
    for(unsigned i =0; i < directories.size(); i++){
        delete directories[i];
    }
    directories.clear();
    description_map.clear();
    floats_map.clear();
}

inline void TIF_CONTAINER::operator=(const TIF_CONTAINER &other){
    char* image_desc_ptr = nullptr;
    this->num_directories = other.num_directories;
    this->width = other.width;
    this->height = other.height;
    this->n_pixels = other.n_pixels;
    this->bits_per_sample = other.bits_per_sample;
    this->filename = other.filename;
    this->samples_per_pixel = other.samples_per_pixel;
    this->desc_num_channels = other.desc_num_channels; //dim C
    this->desc_num_slices = other.desc_num_slices;   //dim Z
    this->desc_num_time = other.desc_num_time;     //dim T
    this->no_order = other.no_order;
    this->description_map = other.description_map;
    this->floats_map = other.floats_map;
    this->channel = other.channel;
    this->slice = other.slice;
    this->time = other.time;
    this->CZT = other.CZT;
    this->directories.resize(0);
    for(unsigned i = 0; i < num_directories; i++){
        directories.push_back(new TIF_DIR);
        directories[i]->initialize(width, height);
        (*directories[i]) = (*other.directories[i]);
    }
}





inline void TIF_CONTAINER::load(std::string _filename){
    filename = _filename;
    std::cout<<"\ngetting properties..";
    getProperties();
    if(num_directories == 0) {std::cout<<"\nnum directories == 0;"; return;}
    std::cout<<"\ngetting description..";
    getDescription();
    std::cout<<"\nsetting counters..";
    setCounters();
    std::cout<<"\nloading data..";
    loadData();
    std::cout<<"\nsorting..";
    sortDirectories();
}

inline void TIF_CONTAINER::clear(){
    for(unsigned i = 0; i < num_directories; i++)
        delete directories[i];
    directories.clear();
    if(image_desc_ptr != nullptr) delete image_desc_ptr;
}

#endif // TIFOPENER_H
