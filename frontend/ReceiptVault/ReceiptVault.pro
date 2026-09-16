QT += core gui widgets charts sql

CONFIG += sdk_no_version_check
CONFIG += c++17

# Command Line Tools' clang doesn't auto-include arm_acle.h before Qt's
# qyieldcpu.h calls the __yield() intrinsic, so -Werror turns the resulting
# implicit-declaration warning into a hard build failure. Force-include the
# header that actually declares it instead of relaxing the warning.
QMAKE_CXXFLAGS += -include arm_acle.h

# Source Files
SOURCES += main.cpp \
           mainwindow.cpp \
           pages/CategoryManagerDialog.cpp \
           pages/DatabaseManager.cpp \
           pages/LoginPage.cpp \
           pages/CreateAccountPage.cpp \
           pages/DashboardPage.cpp \
           pages/ReceiptsPage.cpp \
           pages/AnalyticsPage.cpp \
           pages/BudgetsPage.cpp

# Header Files
HEADERS += mainwindow.h \
           pages/CategoryManagerDialog.h \
           pages/DatabaseManager.h \
           pages/LoginPage.h \
           pages/CreateAccountPage.h \
           pages/DashboardPage.h \
           pages/ReceiptsPage.h \
           pages/AnalyticsPage.h \
           pages/BudgetsPage.h

# Resource Files
RESOURCES += \
    resources.qrc

# Translation Files
TRANSLATIONS += ReceiptVault_en_CA.ts

# Configuration for translations
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

FORMS += \
    AnalyticsPage.ui \
    BudgetsPage.ui \
    CreateAccountPage.ui \
    DashboardPage.ui \
    LoginPage.ui \
    ReceiptsPage.ui
