QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Compiler.cpp \
    GUI.cpp \
    Lexical_Analysis.cpp \
    Syntax_Analysis.cpp \
    assembler.cpp \
    highlighter.cpp \
    linker_l.cpp \
    main.cpp \
    number.cpp \
    simulator.cpp \
    toMIPS.cpp

HEADERS += \
    Compiler.h \
    GUI.h \
    Lcollection.h \
    Lexical_Analysis.h \
    Syntax_Analysis.h \
    assember.h \
    highlighter.h \
    linker_l.h \
    number.h \
    simulator.h \
    toMIPS.h

FORMS += \
    GUI.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    JiuZhang.pro.user \
    Resources/back.png \
    Resources/coding_background.jpg \
    Resources/jz_title.png \
    Resources/niupi.jpg \
    jz_title.png \
    styleSheet/white.qss \
    white.qss
