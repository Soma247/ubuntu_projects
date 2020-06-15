TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        main.cpp
CCFLAG += \
        -pthread \
        -stdlib=libc++ \

LIBS+= -pthread -lc++abi
