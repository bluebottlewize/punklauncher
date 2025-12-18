QT       += core gui widgets quick qml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
CONFIG += debug
CONFIG += console

include(qmltermwidget/lib.pri)

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

LIBS += -lLayerShellQtInterface

SOURCES += \
    src/core/Backend.cpp \
    src/main.cpp \
    src/modules/AliasProvider.cpp \
    src/modules/AnsiParser/AnsiParser.cpp \
    src/modules/AppProvider.cpp \
    src/modules/ListProvider.cpp \
 \    # src/modules/LoadApplications.cpp
    src/modules/Ranker.cpp \
    src/modules/WindowProvider.cpp

HEADERS += \
    src/core/Backend.hpp \
    src/modules/AliasProvider.hpp \
    src/modules/AnsiParser/AnsiParser.hpp \
    src/modules/AppProvider.hpp \
    src/modules/IconProvider.h \
    # src/modules/AppBackend.h \
    src/modules/ListProvider.hpp \
 \    # src/modules/UsageRanker.h
    src/modules/Ranker.hpp \
    src/modules/WindowProvider.hpp

RESOURCES += \
    resources.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    assets/config.json
