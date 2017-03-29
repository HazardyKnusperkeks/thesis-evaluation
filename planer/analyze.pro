SOURCES += \
    main.cpp

CONFIG += c++1z

QT = core

QMAKE_CXXFLAGS += -Wextra -pedantic

HEADERS += \
    ../common/common.hpp \
    ../common/common-qt.hpp

OTHER_FILES += foo.txt
