#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <vector>
#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLTexture>
#include <texture.h>
#include <tifopener.h>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QMouseEvent>
#include <QKeyEvent>
#include <camera.h>
#include <marcher.h>
#include <cloudopener.h>

class GLWidget : public QOpenGLWidget
{
    Q_OBJECT
    QOpenGLFunctions_3_3_Core   *f;
    //things for regular layers drawing
    unsigned char channel_colors[9][3];
    bool colors_changed = false;
    unsigned display_channel = 0;
    bool draw_layers = false;
    QOpenGLShaderProgram        *layerShader;
    QOpenGLBuffer               *layer_VBO;
    QOpenGLVertexArrayObject    *layer_VAO;
        unsigned num_vertices = 0;
    //things for cube marching
    MarchingCube* mcube = nullptr;
    bool draw_march = false;
    bool setup_march = false;
    QOpenGLShaderProgram        *marchShader;
    bool draw_cloud = false;
    bool setup_cloud = false;
    unsigned char iso = 50;
    QOpenGLShaderProgram        *cloudShader;
    QOpenGLBuffer               *cloud_VBO;
    QOpenGLVertexArrayObject    *cloud_VAO;
        unsigned                num_cloud_vertices = 0;
    CloudData cloudData;
    float cloud_properties[2][3] = {{-0.5f, -0.5f, 0.0f},{1.0f/40000.0f, 1.0f/40000.0f, 1.0f/40000.0f}};

    std::vector<TEXTURE*> textures;
        unsigned num_textures;  //textures.size()
        unsigned num_layers;    //how many layers are drawn. either num_textures or that/2 (or/time)
        unsigned num_channels;  //channels used
        //identity
        const QMatrix4x4 E = {1.0f, 0.0f, 0.0f, 0.0f,
                              0.0f, 1.0f, 0.0f, 0.0f,
                              0.0f, 0.0f, 1.0f, 0.0f,
                              0.0f, 0.0f, 0.0f, 1.0f};
    /*view matrices*/
        QMatrix4x4 P_perspective;
        QMatrix4x4 P_ortho;
        QMatrix4x4 V;
    /*camera controls*/
    Camera camera;
    bool first_mouse = true;
    int last_mouse_pos_x = 0;
    int last_mouse_pos_y = 0;
    float rot_angle_x = 0.0f;
    float rot_angle_z = 0.0f;
    float wasd_x = 0.0f;
    float wasd_y = 0.0f;
    float layer_distance = 0.05f;

public:
    TIF_CONTAINER* tif;
private:
    void inline initContext();
    void inline compileShader(QOpenGLShaderProgram *& shader, std::string vertexSrc, std::string fragmentSrc);
    void inline loadVAO();
    void inline loadTextures();
    void inline math();
    void inline calcCamera();

    void inline setupCloud();

    void inline drawLayers();
    void inline drawMarch();
    void inline drawCloud();


public:
    explicit GLWidget(QWidget *parent = nullptr);
    void loadCloud(std::string file);
    void toggleLayers();
    void toggleMarch();
    void toggleCloud();
    void setCloudProperties(float _val[2][3]);
    void setColors(unsigned char _colors[9][3]);
    void setDisplayChannel(int dp);
    void setLayerDistance(double d);
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;
    void calcMarch();
    void recalcLayerDistance();
    ~GLWidget();
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void ISOchange(int value);

signals:
public slots:
    void cleanup();
};





#endif // GLWIDGET_H
