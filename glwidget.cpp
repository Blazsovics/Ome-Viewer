#include "glwidget.h"

namespace shader_source {
std::string layers_vertex_src =
"#version 330 core\n"
"layout (location = 0) in vec4 aData;\n"
"uniform mat4 MVP;\n"
"uniform int DisplayChannel;\n"
"uniform vec3 DisplayTint;\n"
///"uniform int NumChannels;\n"
"out vec2 bTexCoord;\n"
"out int bDisplayChannel;\n"
"out vec3 bDisplayTint;\n"
///"out int bNumChannels;\n"
"void main(){\n"
"   gl_Position = MVP * vec4(aData.x, aData.y, 0.0f, 1.0f);\n"
"   bTexCoord = vec2(aData[2], 1.0f-aData[3]);\n"
"   bDisplayChannel = DisplayChannel;\n"
"   bDisplayTint = DisplayTint;\n"
"}\n"
"\n";


std::string layers_fragment_src =
"#version 330 core\n"
"out vec4 FragColor;\n"
"in vec2 bTexCoord;\n"
"in flat int bDisplayChannel;\n"
"in vec3 bDisplayTint;\n"
"uniform sampler2D TS_BOT;\n"
"uniform sampler2D TS_TOP;\n"
"void main(){\n"
"   int displayLayer = bDisplayChannel / 4;\n"
"   vec4 sampleColor;\n"
"   if(displayLayer == 0)\n"
"       sampleColor = texture(TS_BOT, bTexCoord);\n"
"   else\n"
"       sampleColor = texture(TS_TOP, bTexCoord);\n"
"   int channel = bDisplayChannel % 4;\n"
"   vec4 ChannelColor = vec4(sampleColor[channel], sampleColor[channel], sampleColor[channel], sampleColor[channel]);\n"
"   FragColor = vec4(bDisplayTint, 1.0f) * ChannelColor;\n"
"}\n"
"\n";

std::string march_vertex_src =
"#version 330 core\n"
"layout (location = 0) in vec3 aData;\n"
"uniform mat4 MVP;\n"
"uniform mat4 M;\n"
"uniform vec3 DisplayTint;\n"
"uniform vec3 EyePos;\n"
"out vec3 bDisplayTint;\n"
"out vec3 bWorldPos;\n"
"out vec3 bEyePos;\n"
"void main(){\n"
"   gl_Position = MVP * vec4(aData.x, aData.y, aData.z, 1.0f);\n"
"   vec4 worldPos = M * vec4(aData.x, aData.y, aData.z, 1.0f);\n"
"   bDisplayTint = DisplayTint;\n"
"   bWorldPos = vec3(worldPos.xyz);\n"
"   bEyePos = EyePos.xyz;\n"
"}\n"
"\n";

std::string march_fragment_src =
"#version 330 core\n"
"out vec4 FragColor;\n"
"in vec2 bTexCoord;\n"
"in flat int bDisplayChannel;\n"
"in vec3 bDisplayTint;\n"
"in vec3 bWorldPos;\n"
"in vec3 bEyePos;\n"
"void main(){\n"
"   float dist = distance(bEyePos, bWorldPos);\n"
"   vec3 finalColor = mix(bDisplayTint, vec3(0.0f, 0.0f, 0.0f),dist);"
"   FragColor = vec4(finalColor.rgb, 1.0f);\n"
"}\n"
"\n";

std::string cloud_vertex_src =
"#version 330 core\n"
"layout (location = 0) in vec4 aData;\n"
"uniform mat4 M;\n"
"uniform mat4 MVP;\n"
"out vec2 bTexCoord;\n"
"out float bValue;\n"
"void main(){\n"
"   vec4 worldPos = M * vec4(aData.x, aData.y, aData.z, 1.0f);\n"
"   gl_Position = MVP * vec4(aData.x, aData.y, aData.z, 1.0f);\n"
"   bTexCoord = worldPos.xy;\n"
"   bValue = aData[3];\n"
"}\n"
"\n";

std::string cloud_fragment_src =
"#version 330 core\n"
"out vec4 FragColor;\n"
"in vec2 bTexCoord;\n"
"in float bValue;\n"
"uniform sampler2D TS_ONLY;\n"
"void main(){\n"
"   vec4 colorSample = texture(TS_ONLY, bTexCoord);\n"
"   FragColor = vec4(colorSample.rgb, 1.0f);\n"
"}\n"
"\n";

}

GLWidget::GLWidget(QWidget *parent) : QOpenGLWidget(parent){
    f = nullptr;
    layerShader = nullptr;
    layer_VBO = nullptr;
    layer_VAO = nullptr;
    num_vertices = 0;
    marchShader = nullptr;
    cloudShader = nullptr;
    cloud_VBO = nullptr;
    cloud_VAO = nullptr;
    num_cloud_vertices = 0;
    textures.clear();
    num_textures = 0;
    num_layers = 0;
    num_channels = 0;
    camera = Camera(vec3(0.0f, 0.0f, -3.0f) );
    mcube = new MarchingCube();
    calcCamera();
}

void GLWidget::loadCloud(std::string file)
{
    cloudData.openFile(file);
    setup_cloud = true;
}

void GLWidget::toggleLayers()
{
    draw_layers = !draw_layers;
    this->update();
}

void GLWidget::toggleMarch()
{
    draw_march = !draw_march;
    this->update();
}

void GLWidget::toggleCloud()
{
    draw_cloud = !draw_cloud;
    this->update();
}

void GLWidget::setColors(unsigned char _colors[9][3]){
    for(unsigned i = 0; i < 9; i++){
        for(unsigned c = 0; c < 3; c++){
            channel_colors[i][c]=_colors[i][c];
        }
    }
    colors_changed = true;
}

void GLWidget::setDisplayChannel(int dp){
    this->display_channel = dp-1;
    this->update();
}

void GLWidget::setLayerDistance(double d)
{
    this->layer_distance = d;
    recalcLayerDistance();
    this->update();
}

void GLWidget::cleanup(){
    std::cout<<"\ncleanup runs";
    makeCurrent();
    /*clean textures*/
    for(unsigned i = 0; i < textures.size(); i++){
        if(textures[i]->layer_alpha != nullptr) textures[i]->layer_alpha->destroy();
        if(textures[i]->layer_beta != nullptr) textures[i]->layer_beta->destroy();
        if(textures[i]->march_VBO != nullptr) {textures[i]->march_VBO->destroy(); delete textures[i]->march_VBO;}
        if(textures[i]->march_VAO != nullptr) {textures[i]->march_VAO->destroy(); delete textures[i]->march_VAO;}
        delete textures[i];
    }
    /*clean shaders*/
    if( layerShader != nullptr  ) delete layerShader;
    if( marchShader != nullptr  ) delete marchShader;
    if( cloudShader != nullptr  ) delete cloudShader;
    /*clean vao*/
    if( layer_VAO != nullptr ) { layer_VAO->destroy(); delete layer_VAO;}
    if( layer_VBO != nullptr ) { layer_VBO->destroy(); delete layer_VBO;}
    if( cloud_VAO != nullptr ) { cloud_VAO->destroy(); delete cloud_VAO;}
    if( cloud_VBO != nullptr ) { cloud_VBO->destroy(); delete cloud_VBO;}
    /*clean m_cube*/
    if(mcube != nullptr) {delete mcube;}
    doneCurrent();
}

/*
void GLWidget::initContext(){
    QSurfaceFormat format;
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setOption(QSurfaceFormat::DebugContext);
    //format.setDepthBufferSize(32);
    //format.setStencilBufferSize(0);
    std::cout<<"\nDepth Buffer Size: "<<format.depthBufferSize();
    //format.setSamples(4);
    QSurfaceFormat::setDefaultFormat(format);
    QOpenGLWidget::initializeGL();
    if(QOpenGLContext::currentContext()->create()){}
    else std::cout<<"\ncontext creation bad";
    makeCurrent();
    f = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_3_3_Core>();
    f->initializeOpenGLFunctions();
    if(f == nullptr) std::cout<<"\nF is null";
    //connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &GLWidget::cleanup);
    connect(QOpenGLContext::currentContext(), SIGNAL(aboutToBeDestroyed()), this, SLOT(cleanup()));
    f->glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    f->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    f->glDisable(GL_CULL_FACE);
    f->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    f->glEnable( GL_BLEND );
}
*/
void GLWidget::initContext(){
    QOpenGLWidget::initializeGL();
    if(QOpenGLContext::currentContext()->create()){}
    else std::cout<<"\ncontext creation bad";
    makeCurrent();
    f = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_3_3_Core>();
    f->initializeOpenGLFunctions();
    if(f == nullptr) std::cout<<"\nF is null";
    //connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &GLWidget::cleanup);
    connect(QOpenGLContext::currentContext(), SIGNAL(aboutToBeDestroyed()), this, SLOT(cleanup()));
    f->glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    f->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    f->glDisable(GL_CULL_FACE);
    f->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    f->glEnable( GL_BLEND );
    QSurfaceFormat format;
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setOption(QSurfaceFormat::DebugContext);
    format.setDepthBufferSize(32);
    //format.setStencilBufferSize(0);
    std::cout<<"\nDepth Buffer Size: "<<format.depthBufferSize();
    //format.setSamples(4);
    QSurfaceFormat::setDefaultFormat(format);
}

void GLWidget::compileShader(QOpenGLShaderProgram *& shader, std::string vertexSrc, std::string fragmentSrc){
    QOpenGLShader *vertex_part = new QOpenGLShader ( QOpenGLShader :: Vertex );
    QOpenGLShader *fragment_part = new QOpenGLShader( QOpenGLShader :: Fragment ) ;
    std::string message = "";
    //get shader errors on the console from the glsl compiler
    auto lambda_get_program_error = [&](){
        message = shader->log().toLocal8Bit().constData();
        if(message != "") {std::cout << "\nshaderProgram log:\n"<<message; message = "";}
    };

    auto lambda_get_shader_error = [&](const QOpenGLShader &s){
        std::cout<<"\nget shader error called";
        message = s.log().toLocal8Bit().constData();
        if(message != "") {std::cout << "\nshader log:\n"<<message; message = "";}
    };

    ///Vertex Part
    //try building shader from given source
    if ( !vertex_part->compileSourceCode( vertexSrc.c_str() ) ) {
        lambda_get_shader_error(*vertex_part);
    }
    ///Fragment Part -- same thing, no comments, "vertex"->"fragment"
    if ( !fragment_part->compileSourceCode( fragmentSrc.c_str() ) ) {
        lambda_get_shader_error(*fragment_part);
    }

    shader = new QOpenGLShaderProgram(QOpenGLContext::currentContext());
    shader->addShader( vertex_part );
    lambda_get_program_error();
    shader->addShader( fragment_part );
    lambda_get_program_error();
    if(!shader->link()) {
        lambda_get_program_error();
        return;
    }
    shader->bind();
    if( vertex_part != nullptr     ) delete vertex_part;
    if( fragment_part != nullptr   ) delete fragment_part;
}

void GLWidget::loadVAO(){
    //simplest way of drawing 2 textured triangles with VAO
    const GLfloat vertices[] = {
        -0.5f, -0.5f, 0.0f, 1.0f,
        -0.5f, 0.5f, 0.0f, 0.0f,
        0.5f, -0.5f, 1.0f, 1.0f,

        -0.5f, 0.5f, 0.0f, 0.0f,
        0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, -0.5f, 1.0f, 1.0f
    };
    const unsigned float_count = 24;
    num_vertices = 6;
    makeCurrent();
    if(QOpenGLContext::currentContext() == nullptr) std::cout<<"\nnullptr context\n";

    layer_VAO = new QOpenGLVertexArrayObject(this);
    layer_VAO->create();
    layer_VAO->bind();
    //f->glBindVertexArray(VAO->objectId());

    layer_VBO = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    layer_VBO->setUsagePattern(QOpenGLBuffer::StaticDraw);
    layer_VBO->create();
    layer_VBO->bind();
    //f->glBindVertexArray(VBO->bufferId());
    layer_VBO->allocate(vertices, float_count * sizeof(float));
    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (void*)0); //pack tight
}

void GLWidget::loadTextures(){
    if(this->tif == nullptr) return;
    if(this->tif->num_directories == 0) return;
    TIF_DIR* channels[8];
    auto lambda_reset_channels = [&](){
        for(unsigned i = 0; i < 8; i++) channels[i] = nullptr;
    };
    auto lambda_setup_texture = [&](QOpenGLTexture*& tex, unsigned char* data, unsigned dim_u, unsigned dim_v){
        const QImage qim = QImage(data, dim_u, dim_v, dim_u * 4, QImage::Format_RGBA8888);
        tex = new qtex(qtex::Target2D);
        tex->setMinificationFilter(qtex::Linear);
        tex->setMagnificationFilter(qtex::Linear);
        tex->setBorderColor(0.0f, 0.0f, 0.0f, 0.0f);
        tex->setWrapMode(qtex::ClampToEdge);
        tex->setData(qim,qtex::DontGenerateMipMaps);
        tex->create();
    };
    unsigned anti_pic_counter = tif->num_directories;
    unsigned pic_counter = 0;
    unsigned texture_counter = 0;
    while(anti_pic_counter > 0){
        lambda_reset_channels();
        for(unsigned i = 0; i < tif->desc_num_channels; i++){
            anti_pic_counter--;
            channels[i] = tif->directories[pic_counter];
            pic_counter++;
        }
        textures.push_back(new TEXTURE());
        textures[texture_counter]->initialize(
            channels[0],
            channels[1],
            channels[2],
            channels[3],
            channels[4],
            channels[5],
            channels[6],
            channels[7]
        );
        lambda_setup_texture(textures[texture_counter]->layer_alpha, &textures[texture_counter]->data_a[0], textures[texture_counter]->dim_u, textures[texture_counter]->dim_v);
        if(channels[4] != nullptr)
        lambda_setup_texture(textures[texture_counter]->layer_beta,  &textures[texture_counter]->data_b[0], textures[texture_counter]->dim_u, textures[texture_counter]->dim_v);
        texture_counter++;
    }
    num_textures = textures.size();
}

void GLWidget::math()
{
    recalcLayerDistance();
    float aspect = (float)this->width()/(float)this->height();
    P_perspective = E;
    P_perspective.perspective(80.0f, aspect, 0.0f, 5.0f);
    P_ortho = E;
    P_ortho.ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 20.0f);
}

void GLWidget::calcCamera()
{
    V = E;
    V = V * camera.GetViewMatrix();
}

void GLWidget::setupCloud(){
    num_cloud_vertices = cloudData.data.size()/4;
    std::cout<<"\nsetup:num_cloud_vertices "<<num_cloud_vertices;
    if(cloud_VAO == nullptr){
        cloud_VAO = new QOpenGLVertexArrayObject(this);
        cloud_VAO->create();
    }
    cloud_VAO->bind();

    if(cloud_VBO == nullptr){
        cloud_VBO = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
        cloud_VBO->setUsagePattern(QOpenGLBuffer::StaticDraw);
        cloud_VBO->create();
    }
    cloud_VBO->bind();
    layer_VBO->allocate(cloudData.data.data(), num_cloud_vertices * sizeof(float) * 4);
    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (void*)0); //pack tight
    cloudData.data.clear();
    f->glPointSize(1.0f);
}

void GLWidget::setCloudProperties(float _val[2][3]){
    for(unsigned i = 0; i < 2; i++){
        for(unsigned c = 0; c < 3; c++){
            cloud_properties[i][c]=_val[i][c];
        }
    }
}

void GLWidget::initializeGL(){
    ///automagically called before the first paint
    initContext();
    compileShader(layerShader, shader_source::layers_vertex_src, shader_source::layers_fragment_src);
    layerShader->setUniformValue("TS_BOT", 0);
    layerShader->setUniformValue("TS_TOP", 1);
    compileShader(marchShader, shader_source::march_vertex_src, shader_source::march_fragment_src);
    marchShader->setUniformValue("TS_UPPER", 0);
    marchShader->setUniformValue("TS_LOWER", 1);
    compileShader(cloudShader, shader_source::cloud_vertex_src, shader_source::cloud_fragment_src);
    cloudShader->setUniformValue("TS_ONLY", 0);
    cloudShader->setUniformValue("M", E);
    cloudShader->setUniformValue("VP", E);
    loadTextures();
    loadVAO();
    math();
}

void GLWidget::drawLayers(){
    if(!draw_layers) return;
    if(layerShader != nullptr) layerShader->bind();
    layerShader->setUniformValue("DisplayChannel", display_channel);
    layerShader->setUniformValue("DisplayTint", QVector3D(
                                     (float)channel_colors[display_channel][0]/255.0f,
                                     (float)channel_colors[display_channel][1]/255.0f,
                                     (float)channel_colors[display_channel][2]/255.0f
                                   ));
    layer_VAO->bind();
    if(num_textures == 0) return;
    f->glDisable( GL_CULL_FACE );
    f->glDisable( GL_DEPTH_TEST );
    for(int i = num_textures-1; 0<=i; i--){
        layerShader->setUniformValue("MVP", P_perspective * V * textures[i]->Model);
        if(textures[i]->layer_alpha != nullptr) textures[i]->layer_alpha->bind(0);
        if(textures[i]->layer_beta  != nullptr) textures[i]-> layer_beta->bind(1);
        f->glDrawArrays(GL_TRIANGLES, 0, num_vertices);
    }
    f->glEnable( GL_DEPTH_TEST );
}

void GLWidget::drawMarch(){
    if(mcube == nullptr) return;
    if(setup_march){
        std::cout<<"\nsetup_march";
        mcube->layer_distance = this->layer_distance;
        char iso = 50;
        for(unsigned i = 0; i < textures.size()-1; i++){
            mcube->MarchLayer(
                        textures[i+0]->dirs[display_channel],
                        textures[i+1]->dirs[display_channel], iso);
            mcube->generate(f, textures[i]->march_VAO, textures[i]->march_VBO, textures[i]->march_num_vertices);
            std::cout<<"\n\tgenerated "<<i<<" vertices: "<<textures[i]->march_num_vertices;
        }
        setup_march = false;
    }
    if(draw_march == false) return;
    if(marchShader != nullptr) marchShader->bind(); else return;
    marchShader->setUniformValue("DisplayChannel", display_channel);
    marchShader->setUniformValue("DisplayTint", QVector3D(
                                     (float)channel_colors[display_channel][0]/255.0f,
                                     (float)channel_colors[display_channel][1]/255.0f,
                                     (float)channel_colors[display_channel][2]/255.0f
                                   ));
    marchShader->setUniformValue("EyePos", camera.Position);
    /*
    f->glDisable( GL_DEPTH_TEST );
    marchShader->setUniformValue("MVP", P_perspective * V * (textures[0]->Model));//offset by corresponding bottom texture M_Matrix
    marchShader->setUniformValue("M", (textures[0]->Model));
    if(textures[0]->march_VAO != nullptr) {
        textures[0]->march_VAO->bind();
        f->glDrawArrays(GL_TRIANGLES, 0, textures[0]->march_num_vertices);
    }
    */


    f->glClear( GL_DEPTH_BUFFER_BIT );
    f->glDisable( GL_CULL_FACE );
    f->glDisable( GL_DEPTH_TEST );
    for(int i = num_textures-2; 0<=i; i--){
        marchShader->setUniformValue("MVP", P_perspective * V * (textures[i+0]->Model));//offset by corresponding bottom texture M_Matrix
        marchShader->setUniformValue("M", (textures[i+0]->Model));
        /*
        if(display_channel < 4){
            if(textures[i+0]->layer_beta != nullptr) textures[i+0]->layer_alpha->bind(0);
            if(textures[i+1]->layer_beta != nullptr) textures[i+1]->layer_alpha->bind(1);
        } else {
            if(textures[i+0]->layer_beta != nullptr) textures[i+0]->layer_alpha->bind(0);
            if(textures[i+1]->layer_beta != nullptr) textures[i+1]->layer_alpha->bind(1);
        }
        */
        if(textures[i]->march_VAO != nullptr) {
            textures[i]->march_VAO->bind();
            f->glDrawArrays(GL_TRIANGLES, 0, textures[i]->march_num_vertices);
        } else return;
    }

}

void GLWidget::drawCloud(){
    if(setup_cloud){
        setupCloud();
        setup_cloud = false;
    }
    if(!draw_cloud) return;

    std::cout<<"\ndrawing cloud "<<num_cloud_vertices;
    QMatrix4x4 CloudModel = E;
    CloudModel.translate(cloud_properties[0][0], cloud_properties[0][1], cloud_properties[0][2]);
    CloudModel.scale(cloud_properties[1][0], cloud_properties[1][1], cloud_properties[1][2]);
    calcCamera();
    if(cloudShader != nullptr) cloudShader->bind();
    else return;
    cloudShader->setUniformValue("M", CloudModel);
    cloudShader->setUniformValue("VP", V);
    cloudShader->setUniformValue("MVP", P_perspective * V * CloudModel);
    if(cloud_VAO != nullptr) cloud_VAO->bind();
    else return;
    if(textures[0]->layer_alpha != nullptr) textures[0]->layer_alpha->bind();
    f->glPointSize(1.0f);
    f->glDrawArrays(GL_POINTS, 0, num_cloud_vertices);
}

void GLWidget::paintGL()
{
    if(colors_changed){
        f->glClearColor((float)channel_colors[8][0]/255.0f, (float)channel_colors[8][1]/255.0f, (float)channel_colors[8][2]/255.0f, 1.0f);
        colors_changed = false;
    }
    f->glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );
    calcCamera();
    drawLayers();
    drawCloud();
    drawMarch();
    QOpenGLContext::currentContext()->swapBuffers(QOpenGLContext::currentContext()->surface());
}

void GLWidget::resizeGL(int w, int h)
{
    f->glViewport( 0, 0, w, h );
    float aspect = (float)this->width()/(float)this->height();
    P_perspective.perspective(90.0f, aspect, 0.0f, 5.0f);
}

void GLWidget::calcMarch()
{
    setup_march = true;
}

void GLWidget::recalcLayerDistance()
{
    for(unsigned i = 0; i < num_textures; i++){
        textures[i]->Model = E;
        textures[i]->Model.translate(0.0f, 0.0f, i * layer_distance);
    }
}

GLWidget::~GLWidget()
{
    cleanup();
};

void GLWidget::mouseMoveEvent(QMouseEvent *event){
    //std::cout<<"\ncamera pos: "<<camera.Position.x()<<' '<<camera.Position.y()<<' '<<camera.Position.z();
    //std::cout<<"\ncamera Front: "<<camera.Front.x()<<' '<<camera.Front.y()<<' '<<camera.Front.z();
    if (first_mouse)
    {
        last_mouse_pos_x = event->x();
        last_mouse_pos_y = event->y();
        first_mouse = false;
    }
    float xoffset = event->x() - last_mouse_pos_x;
    float yoffset = last_mouse_pos_y - event->y(); // reversed since y-coordinates go from bottom to top
    last_mouse_pos_x = event->x();
    last_mouse_pos_y = event->y();
    camera.ProcessMouseMovement(xoffset, yoffset);
    this->update();
}

void GLWidget::keyPressEvent(QKeyEvent* event){
    double deltaTime = 0.01;
    switch(event->key()) {
        case Qt::Key_W:{camera.ProcessKeyboard(BACKWARD, deltaTime); break;}
        case Qt::Key_S:{camera.ProcessKeyboard(FORWARD, deltaTime); break;}
        case Qt::Key_A:{camera.ProcessKeyboard(LEFT, deltaTime); break;}
        case Qt::Key_D:{camera.ProcessKeyboard(RIGHT, deltaTime); break;}
        case Qt::Key_Space:{camera.ProcessKeyboard(UP, deltaTime); break;}
        case Qt::Key_C:{camera.ProcessKeyboard(DOWN, deltaTime); break;}
        case Qt::Key_Q:{if(display_channel==0) display_channel = 7; else display_channel--; break;}
        case Qt::Key_E:{if(display_channel==7) display_channel = 0; else display_channel++; break;}
        default: return;
    }
    this->update();
}

void GLWidget::ISOchange(int value){ this->iso = value; }

void GLWidget::mousePressEvent(QMouseEvent* event){
    if(event == nullptr) return;
    first_mouse = true;
}

void GLWidget::mouseReleaseEvent(QMouseEvent* event){
    if(event == nullptr) return;
    first_mouse = false;
}
