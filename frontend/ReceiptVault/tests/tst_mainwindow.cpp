#include <QtTest>
#include "mainwindow.h"

class TestMainWindow : public QObject
{
    Q_OBJECT

private slots:
    void normalizeReceiptDate_flipsMonthDayYear();
    void normalizeReceiptDate_flipsDayMonthYear();
    void normalizeReceiptDate_expandsTwoDigitYear();
    void normalizeReceiptDate_leavesCorrectFormatAlone();
    void normalizeReceiptDate_rejectsGarbage();
};

void TestMainWindow::normalizeReceiptDate_flipsMonthDayYear()
{
    QCOMPARE(MainWindow::normalizeReceiptDate("01-15-2026"), QString("2026-01-15"));
}

void TestMainWindow::normalizeReceiptDate_flipsDayMonthYear()
{
    QCOMPARE(MainWindow::normalizeReceiptDate("25-01-2026"), QString("2026-01-25"));
}

void TestMainWindow::normalizeReceiptDate_expandsTwoDigitYear()
{
    QCOMPARE(MainWindow::normalizeReceiptDate("01-15-26"), QString("2026-01-15"));
}

void TestMainWindow::normalizeReceiptDate_leavesCorrectFormatAlone()
{
    QCOMPARE(MainWindow::normalizeReceiptDate("2026-01-15"), QString("2026-01-15"));
}

void TestMainWindow::normalizeReceiptDate_rejectsGarbage()
{
    QCOMPARE(MainWindow::normalizeReceiptDate("garbage"), QString(""));
}

QTEST_APPLESS_MAIN(TestMainWindow)
#include "tst_mainwindow.moc"
