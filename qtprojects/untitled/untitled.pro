TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt
QMAKE_LFLAGS += -pthread -stlib=lc++ -Wall -ggdb -lc++abi
SOURCES += main.cpp
