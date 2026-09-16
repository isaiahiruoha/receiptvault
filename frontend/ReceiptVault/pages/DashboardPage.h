#ifndef DASHBOARDPAGE_H
#define DASHBOARDPAGE_H

#include <QWidget> // for QWidget base class
#include <QtCharts/QChartView> // for displaying charts
#include <QLabel> // for displaying text and information
#include <QPushButton> // for navigation buttons

namespace Ui {
class DashboardPage; // forward declaration of ui class
}

class DashboardPage : public QWidget
{
    Q_OBJECT // needed for signals and slots

public:
    explicit DashboardPage(QWidget *parent = nullptr); // constructor to set up the widget
    ~DashboardPage(); // destructor to clean up resources

    // updates the dashboard with user data
    void updateDashboard(
        int totalReceipts, // total number of receipts
        double totalSpending, // total spending amount
        double avgMonthlySpending, // average monthly spending
        QString topCategory, // top spending category
        const QList<QPair<QString, double>> &spendingData // data for spending distribution
        );

signals:
    void navigateToReceipts(); // signal to navigate to the receipts page
    void navigateToAnalytics(); // signal to navigate to the analytics page
    void navigateToBudgets(); // signal to navigate to the budgets page
    void logoutRequested(); // signal to log out the user

private:
    Ui::DashboardPage *ui; // pointer to ui elements for this page
};

#endif
