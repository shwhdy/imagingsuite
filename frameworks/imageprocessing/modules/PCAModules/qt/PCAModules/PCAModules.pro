#-------------------------------------------------
#
# Project created by QtCreator 2013-12-30T19:34:12
#
#-------------------------------------------------

QT       += core

TARGET = PCAModules
TEMPLATE = lib
CONFIG += c++11

CONFIG(release, debug|release): DESTDIR = $$PWD/../../../../../../../lib
else:CONFIG(debug, debug|release): DESTDIR = $$PWD/../../../../../../../lib/debug

unix:!macx {
    QMAKE_CXXFLAGS += -fopenmp -fPIC -O2
    QMAKE_LFLAGS += -lgomp
    LIBS += -lgomp
}

unix:macx {
    QMAKE_MAC_SDK = macosx10.12
    QMAKE_CXXFLAGS += -fPIC -O2
    INCLUDEPATH += /opt/local/include
    QMAKE_LIBDIR += /opt/local/lib
}


DEFINES += PCAMODULES_LIBRARY

SOURCES += ../../src/pcamodules.cpp \
    ../../src/PCAFilterModule.cpp \
    ../../src/stdafx.cpp

HEADERS += \
    ../../src/PCAFilterModule.h \
    ../../src/targetver.h \
    ../../src/stdafx.h \
    ../../src/PCAModules_global.h \
    ../../src/pcamodules.h

unix:INCLUDEPATH += $$PWD/../../../../../../external/src/linalg

symbian {
    MMP_RULES += EXPORTUNFROZEN
    TARGET.UID3 = 0xEFEEBACE
    TARGET.CAPABILITY = 
    TARGET.EPOCALLOWDLLDATA = 1
    addFiles.sources = PCAModules.dll
    addFiles.path = !:/sys/bin
    DEPLOYMENT += addFiles
}

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target

    LIBS += -ltiff -lxml2
    INCLUDEPATH += /usr/include/libxml2
}

macx: {
INCLUDEPATH += $$PWD/../../../../../../external/mac/include $$PWD/../../../../../../external/mac/include/hdf5 $$PWD/../../../../../../external/mac/include/nexus
DEPENDPATH += $$PWD/../../../../../../external/mac/include $$PWD/../../../../../../external/mac/include/hdf5 $$PWD/../../../../../../external/mac/include/nexus
LIBS += -L$$PWD/../../../../../../external/mac/lib/ -lNeXus.1.0.0 -lNeXusCPP.1.0.0
}

CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../../../../lib
else:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../../../../lib/debug/

LIBS += -lkipl -lModuleConfig -lProcessFramework

INCLUDEPATH += $$PWD/../../../../../../frameworks/imageprocessing/ProcessFramework/include
DEPENDPATH += $$PWD/../../../../../../frameworks/imageprocessing/ProcessFramework/include

INCLUDEPATH += $$PWD/../../../../../../core/modules/ModuleConfig/include
DEPENDPATH += $$PWD/../../../../../../core/modules/ModuleConfig/include

INCLUDEPATH += $$PWD/../../../../../../core/kipl/kipl/include
DEPENDPATH += $$PWD/../../../../../../core/kipl/kipl/include
