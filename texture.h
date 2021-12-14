#ifndef TEXTURE_H
#define TEXTURE_H

#include <tifopener.h>
#include <QOpenGLFunctions_3_3_Core>
#include <QMatrix4x4>
#include <QVector4D>
#include <QOpenGLTexture>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#define vec4 QVector4D
#define mat4 QMatrix4x4
#define qtex QOpenGLTexture

struct TEXTURE{
    //can contain 8 channels of data = 2 image textures
    unsigned dim_u, dim_v;
    unsigned layer_a;
    unsigned char* data_a;
    unsigned layer_b;
    unsigned char* data_b;
    unsigned num_channels;
    QOpenGLTexture *layer_alpha;
    QOpenGLTexture *layer_beta;
    mat4 Model;
    QOpenGLBuffer* march_VBO = nullptr;
    QOpenGLVertexArrayObject* march_VAO = nullptr;
    unsigned march_num_vertices = 0;
    TIF_DIR* dirs[8];
    TEXTURE();
    ~TEXTURE();

    void initialize(TIF_DIR* a_0,
                    TIF_DIR* a_1 = nullptr,
                    TIF_DIR* a_2 = nullptr,
                    TIF_DIR* a_3 = nullptr,
                    TIF_DIR* b_0 = nullptr,
                    TIF_DIR* b_1 = nullptr,
                    TIF_DIR* b_2 = nullptr,
                    TIF_DIR* b_3 = nullptr);

    unsigned char& index(unsigned pixel_u, unsigned pixel_v, unsigned channel, unsigned layer = 0);
};

inline TEXTURE::TEXTURE()
{
    data_a = nullptr;
    data_b = nullptr;
    num_channels = 0;
    dim_u = 0;
    dim_v = 0;
    layer_alpha = nullptr;
    layer_beta = nullptr;
    march_VAO = nullptr;
    march_VBO = nullptr;
}

inline TEXTURE::~TEXTURE()
{
    if(data_a != nullptr) delete [] data_a;
    if(data_b != nullptr) delete [] data_b;
    if(march_VAO != nullptr) delete march_VAO;
    if(march_VBO != nullptr) delete march_VBO;
}

inline void TEXTURE::initialize(TIF_DIR *a_0, TIF_DIR *a_1, TIF_DIR *a_2, TIF_DIR *a_3, TIF_DIR *b_0, TIF_DIR *b_1, TIF_DIR *b_2, TIF_DIR *b_3)
{
    if(a_0 == nullptr) return;
    else num_channels = 1;
    if(a_1 != nullptr) num_channels++;
    if(a_2 != nullptr) num_channels++;
    if(a_3 != nullptr) num_channels++;
    if(b_0 != nullptr) num_channels++;
    if(b_1 != nullptr) num_channels++;
    if(b_2 != nullptr) num_channels++;
    if(b_3 != nullptr) num_channels++;
    dirs[0] = a_0;
    dirs[1] = a_1;
    dirs[2] = a_2;
    dirs[3] = a_3;
    dirs[4] = b_0;
    dirs[5] = b_1;
    dirs[6] = b_2;
    dirs[7] = b_3;
    dim_u = a_0->dim_u;
    dim_v = a_0->dim_v;


    auto lambda_fill_layer = [&](TIF_DIR* channel_data, unsigned char channel = 0, unsigned char layer = 0){
        if(channel_data != nullptr) {
            for(unsigned v = 0; v < dim_v; v++){
                for(unsigned u = 0; u < dim_v; u++){
                    index(u, v, channel, layer) = channel_data->index(u,v);
                }
            }
        } else {
            //fill with 0-s if no data is given
            for(unsigned v = 0; v < dim_v; v++){
                for(unsigned u = 0; u < dim_v; u++){
                    index(u, v, channel, layer) = 0;
                }
            }
        }
    };

    /*init layer a*/{
        data_a = new unsigned char [dim_u * 4 * dim_v];
        lambda_fill_layer(a_0, 0, 0);
        if(num_channels > 1) lambda_fill_layer(a_1, 1, 0);
        if(num_channels > 2) lambda_fill_layer(a_2, 2, 0);
        if(num_channels > 3) lambda_fill_layer(a_3, 3, 0);
    }

    /*init layer b*/if(num_channels > 4){
        data_b = new unsigned char [dim_u * 4 * dim_v];
        lambda_fill_layer(b_0, 0, 1);
        if(num_channels > 5) lambda_fill_layer(b_1, 1, 1);
        if(num_channels > 6) lambda_fill_layer(b_2, 2, 1);
        if(num_channels > 7) lambda_fill_layer(b_3, 3, 1);
    }

    /*
    std::thread T_a0(lambda_fill_layer, a_0, 0, 0);
    std::thread T_a1(lambda_fill_layer, a_1, 1, 0);
    std::thread T_a2(lambda_fill_layer, a_2, 2, 0);
    std::thread T_a3(lambda_fill_layer, a_3, 3, 0);
    if(b_0 != nullptr){
        std::thread T_b0(lambda_fill_layer, b_0, 0, 1);
        std::thread T_b1(lambda_fill_layer, b_1, 1, 1);
        std::thread T_b2(lambda_fill_layer, b_2, 2, 1);
        std::thread T_b3(lambda_fill_layer, b_3, 3, 1);
        T_b0.join();
        T_b1.join();
        T_b2.join();
        T_b3.join();
    }
    T_a0.join();
    T_a1.join();
    T_a2.join();
    T_a3.join();
    */
}

inline unsigned char &TEXTURE::index(unsigned pixel_u, unsigned pixel_v, unsigned channel, unsigned layer){
    //size_of_row = 4 * dim_u
    //skip vertical * size_of_row = pixel_v * 4 * dim_u
    //step ~right pixel_u amount += pixel_u * 4
    // * 4 everywhere because its a byte array, and we want pixels
    //layer = alpha or beta texture
    if(layer == 0){
        /* texture alpha */
        if(pixel_u < dim_u && pixel_v < dim_v){
            return data_a[pixel_v * dim_u * 4 + pixel_u * 4 + channel];
        } else return data_a[0];
    } else {
        /* texture beta */
        if(pixel_u < dim_u && pixel_v < dim_v){
            return data_b[pixel_v * dim_u * 4 + pixel_u * 4 + channel];
        } else return data_b[0];
    }

}



#endif // TEXTURE_H
