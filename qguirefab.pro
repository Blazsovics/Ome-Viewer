QT       += \
        core \
        gui \
        printsupport \
        opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
CONFIG += console

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    glwidget.cpp \
    histogram.cpp \
    main.cpp \
    mainwindow.cpp \
    qcustomplot.cpp \
    slideshowwindow.cpp \
    window3d.cpp

HEADERS += \
    camera.h \
    cloudopener.h \
    glwidget.h \
    histogram.h \
    mainwindow.h \
    marcher.h \
    qcustomplot.h \
    slideshowwindow.h \
    texture.h \
    tifopener.h \
    window3d.h

FORMS += \
    mainwindow.ui \
    slideshowwindow.ui \
    window3d.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/libs/qlibtif/libtiff/ -llibtiff.dll
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/libs/qlibtif/libtiff/ -llibtiff.dll
else:unix:!macx: LIBS += -L$$PWD/libs/qlibtif/libtiff/ -llibtiff.dll

INCLUDEPATH += $$PWD/libs/qlibtif/libtiff
DEPENDPATH += $$PWD/libs/qlibtif/libtiff

LIBS += -L$$PWD/libs/opencv/lib/ \
    -lopencv_core454 \
    -lopencv_calib3d454 \
    -lopencv_dnn454 \
    -lopencv_features2d454 \
    -lopencv_flann454 \
    -lopencv_gapi454 \
    -lopencv_highgui454 \
    -lopencv_imgcodecs454 \
    -lopencv_imgproc454 \
    -lopencv_ml454 \
    -lopencv_objdetect454 \
    -lopencv_photo454 \
    -lopencv_stitching454 \
    -lopencv_video454 \
    -lopencv_videoio454

LIBS += -L$$PWD/libs/opencv/bin/ \
    -lopencv_core454.dll \
    -lopencv_calib3d454.dll \
    -lopencv_dnn454.dll \
    -lopencv_features2d454.dll \
    -lopencv_flann454.dll \
    -lopencv_gapi454.dll \
    -lopencv_highgui454.dll \
    -lopencv_imgcodecs454.dll \
    -lopencv_imgproc454.dll \
    -lopencv_ml454.dll \
    -lopencv_objdetect454.dll \
    -lopencv_photo454.dll \
    -lopencv_stitching454.dll \
    -lopencv_video454.dll \
    -lopencv_videoio454.dll

INCLUDEPATH += $$PWD/libs/opencv/include
DEPENDPATH += $$PWD/libs/opencv/include

