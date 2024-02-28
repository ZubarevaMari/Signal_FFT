QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets sql network printsupport

CONFIG += c++14

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    extern/dbparse/database.cpp \
    extern/dbparse/dataproc.cpp \
    extern/dbparse/datasource.cpp \
    extern/dbparse/network.cpp \
    fft_calc.cpp \
    main.cpp \
    mainwindow.cpp \
    qcustomplot.cpp

HEADERS += \
    extern/dbparse/database.h \
    extern/dbparse/dataproc.h \
    extern/dbparse/datasource.h \
    extern/dbparse/network.h \
    extern/fftw/fftw3.h \
    fft_calc.h \
    mainwindow.h \
    qcustomplot.h

FORMS += \
    mainwindow.ui

LIBS+= -lws2_32 -lwpcap -lsetupapi -lfftw3-3

contains(QT_ARCH, i386) {
    LIBS += -L$$PWD/extern/winpcap/Lib/ -lwpcap # 32-bit
}else {
    LIBS += -L$$PWD/extern/winpcap/ -lwpcap # 64-bit
}

INCLUDEPATH += $$PWD/extern/winpcap/Include
DEPENDPATH += $$PWD/extern/winpcap/Include

LIBS+= -L$$PWD/extern/fftw -lfftw3-3

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
