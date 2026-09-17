QT += testlib sql
CONFIG += testcase c++17
TARGET = tst_databasemanager

QMAKE_CXXFLAGS += -include arm_acle.h
INCLUDEPATH += ../pages

SOURCES += tst_databasemanager.cpp \
           ../pages/DatabaseManager.cpp
HEADERS += ../pages/DatabaseManager.h
