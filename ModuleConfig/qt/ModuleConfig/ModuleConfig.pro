#-------------------------------------------------
#
# Project created by QtCreator 2013-05-04T12:48:45
#
#-------------------------------------------------

QT       -= gui

TARGET = ModuleConfig
TEMPLATE = lib
unix {
unix:!macx {
    QMAKE_CXXFLAGS += -fopenmp
    QMAKE_LFLAGS += -lgomp
    LIBS += -lgomp
}
    LIBS += -L/usr/lib -lxml2
    INCLUDEPATH += /usr/include/libxml2

}

unix:macx {
    LIBS += -L/usr/lib -lxml2
    INCLUDEPATH += /usr/include/libxml2
}

win32 {
    contains(QMAKE_HOST.arch, x86_64):{
    QMAKE_LFLAGS += /MACHINE:X64
    }
    INCLUDEPATH += ../../../../../external/include
    LIBPATH += ../../../../../external/lib64
    LIBS += -llibxml2_dll
    QMAKE_CXXFLAGS += /openmp /O2
}

DEFINES += MODULECONFIG_LIBRARY

SOURCES += \
    ../../src/stdafx.cpp \
    ../../src/ProcessModuleBase.cpp \
    ../../src/ParameterHandling.cpp \
    ../../src/ModuleItemBase.cpp \
    ../../src/ModuleException.cpp \
    ../../src/ModuleConfig.cpp \
    ../../src/dllmain.cpp \
    ../../src/ConfigBase.cpp

HEADERS +=\
        ModuleConfig_global.h \
    ../../include/targetver.h \
    ../../include/ProcessModuleBase.h \
    ../../include/ParameterHandling.h \
    ../../include/ModuleItemBase.h \
    ../../include/ModuleException.h \
    ../../include/ModuleConfig.h \
    ../../include/ConfigBase.h \
    ../../src/stdafx.h

symbian {
    MMP_RULES += EXPORTUNFROZEN
    TARGET.UID3 = 0xECD31538
    TARGET.CAPABILITY = 
    TARGET.EPOCALLOWDLLDATA = 1
    addFiles.sources = ModuleConfig.dll
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
}

win32:CONFIG(release, debug|release):    LIBS += -L$$PWD/../../../../../kipl/trunk/kipl/build-kipl-Qt5-Release/release -lkipl
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../../kipl/trunk/kipl/build-kipl-Qt5-Debug/debug -lkipl
else:unix:CONFIG(release, debug|release) LIBS += -L$$PWD/../../../../../kipl/trunk/kipl/build-kipl-Qt5-Release/ -lkipl
else:unix:CONFIG(debug, debug|release)   LIBS += -L$$PWD/../../../../../kipl/trunk/kipl/build-kipl-Qt5-Debug/ -lkipl

INCLUDEPATH += $$PWD/../../../../../kipl/trunk/kipl/include
DEPENDPATH += $$PWD/../../../../../kipl/trunk/kipl/include
