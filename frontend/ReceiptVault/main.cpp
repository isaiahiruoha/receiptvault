#include "mainwindow.h"
#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Stable identity so QStandardPaths resolves a consistent data directory
    // (~/Library/Application Support/ReceiptVault on macOS).
    QApplication::setApplicationName("ReceiptVault");

    // Set the application icon
    a.setWindowIcon(QIcon(":/icons/ReceiptVault.png"));

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "ReceiptVault_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    MainWindow w;
    w.show();
    return a.exec();
}
