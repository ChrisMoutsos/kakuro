QT       += core widgets

TARGET = kakuro
TEMPLATE = app

RESOUCES = resources.qrc

SOURCES += main.cpp\
        mainwindow.cpp \
    cell.cpp \
    puzzleboard.cpp \
    combohelperdialog.cpp

HEADERS  += mainwindow.h \
    cell.h \
    puzzleboard.h \
    common.h \
    combohelperdialog.h

FORMS    +=

RESOURCES += \
    resources.qrc

CONFIG += c++11
