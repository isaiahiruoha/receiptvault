QT += widgets charts testlib
CONFIG += testcase c++17
TARGET = tst_dashboardpage

QMAKE_CXXFLAGS += -include arm_acle.h
INCLUDEPATH += ../pages

SOURCES += tst_dashboardpage.cpp \
           ../pages/DashboardPage.cpp
HEADERS += ../pages/DashboardPage.h
FORMS += ../DashboardPage.ui
