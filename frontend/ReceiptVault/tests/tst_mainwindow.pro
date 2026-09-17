QT += widgets charts sql testlib
CONFIG += testcase c++17
TARGET = tst_mainwindow

QMAKE_CXXFLAGS += -include arm_acle.h
INCLUDEPATH += .. ../pages

SOURCES += tst_mainwindow.cpp \
           ../mainwindow.cpp \
           ../pages/LoginPage.cpp \
           ../pages/CreateAccountPage.cpp \
           ../pages/DashboardPage.cpp \
           ../pages/ReceiptsPage.cpp \
           ../pages/AnalyticsPage.cpp \
           ../pages/BudgetsPage.cpp \
           ../pages/CategoryManagerDialog.cpp \
           ../pages/DatabaseManager.cpp
HEADERS += ../mainwindow.h \
           ../pages/LoginPage.h \
           ../pages/CreateAccountPage.h \
           ../pages/DashboardPage.h \
           ../pages/ReceiptsPage.h \
           ../pages/AnalyticsPage.h \
           ../pages/BudgetsPage.h \
           ../pages/CategoryManagerDialog.h \
           ../pages/DatabaseManager.h
FORMS += ../LoginPage.ui \
          ../CreateAccountPage.ui \
          ../DashboardPage.ui \
          ../ReceiptsPage.ui \
          ../AnalyticsPage.ui \
          ../BudgetsPage.ui
