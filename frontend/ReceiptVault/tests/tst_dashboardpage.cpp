#include <QtTest>
#include "DashboardPage.h"

class TestDashboardPage : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void logoutButton_emitsLogoutRequested();
    void viewReceiptsButton_emitsNavigateToReceipts();
    void viewBudgetsButton_emitsNavigateToBudgets();
    void viewAnalyticsButton_emitsNavigateToAnalytics();

private:
    DashboardPage *page = nullptr;
};

void TestDashboardPage::init()
{
    page = new DashboardPage();
}

void TestDashboardPage::cleanup()
{
    delete page;
    page = nullptr;
}

void TestDashboardPage::logoutButton_emitsLogoutRequested()
{
    QSignalSpy spy(page, &DashboardPage::logoutRequested);
    QPushButton *button = page->findChild<QPushButton *>("logoutButton");
    QVERIFY(button);
    QTest::mouseClick(button, Qt::LeftButton);
    QCOMPARE(spy.count(), 1);
}

void TestDashboardPage::viewReceiptsButton_emitsNavigateToReceipts()
{
    QSignalSpy spy(page, &DashboardPage::navigateToReceipts);
    QPushButton *button = page->findChild<QPushButton *>("viewReceiptsButton");
    QVERIFY(button);
    QTest::mouseClick(button, Qt::LeftButton);
    QCOMPARE(spy.count(), 1);
}

void TestDashboardPage::viewBudgetsButton_emitsNavigateToBudgets()
{
    QSignalSpy spy(page, &DashboardPage::navigateToBudgets);
    QPushButton *button = page->findChild<QPushButton *>("viewBudgetsButton");
    QVERIFY(button);
    QTest::mouseClick(button, Qt::LeftButton);
    QCOMPARE(spy.count(), 1);
}

void TestDashboardPage::viewAnalyticsButton_emitsNavigateToAnalytics()
{
    QSignalSpy spy(page, &DashboardPage::navigateToAnalytics);
    QPushButton *button = page->findChild<QPushButton *>("viewAnalyticsButton");
    QVERIFY(button);
    QTest::mouseClick(button, Qt::LeftButton);
    QCOMPARE(spy.count(), 1);
}

QTEST_MAIN(TestDashboardPage)
#include "tst_dashboardpage.moc"
