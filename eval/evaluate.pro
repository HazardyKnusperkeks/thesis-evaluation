SOURCES += \
    main.cpp \
    qcustomplot/qcustomplot.cpp \
    datenwidget.cpp \
    roboterplot.cpp \
    produktplot.cpp \
    encodingfenster.cpp \
    hauptfenster.cpp \
    zeitbismodellplot.cpp \
    graphdialog.cpp \
    parseclips.cpp

CONFIG += c++1z

QT = core gui widgets printsupport

QMAKE_CXXFLAGS += -Wextra -pedantic

HEADERS += \
    ../common/common-qt.hpp \
    qcustomplot/qcustomplot.h \
    datenwidget.hpp \
    structs.hpp \
    roboterplot.hpp \
    produktplot.hpp \
    ../common/common.hpp \
    encodingfenster.hpp \
    hauptfenster.hpp \
    zeitbismodellplot.hpp \
    graphdialog.hpp \
    graphlegende.hpp \
    parseclips.hpp
