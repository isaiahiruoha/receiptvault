#include "DashboardPage.h"
#include "ui_DashboardPage.h"
#include <QtCharts/QBarSet> // for bar set
#include <QtCharts/QBarSeries> // for bar series
#include <QtCharts/QPieSeries> // for pie chart series
#include <QtCharts/QBarCategoryAxis> // for bar chart axis
#include <QtCharts/QValueAxis> // for value axis

// constructor
DashboardPage::DashboardPage(QWidget *parent)
    : QWidget(parent), ui(new Ui::DashboardPage), currentUserId(-1) // initialize ui and set user id to -1
{
    ui->setupUi(this); // set up the ui from the .ui file

    // connect buttons to their respective signals
    connect(ui->viewReceiptsButton, &QPushButton::clicked, this, &DashboardPage::navigateToReceipts);
    connect(ui->viewAnalyticsButton, &QPushButton::clicked, this, &DashboardPage::navigateToAnalytics);
    connect(ui->viewBudgetsButton, &QPushButton::clicked, this, &DashboardPage::navigateToBudgets);
    connect(ui->logoutButton, &QPushButton::clicked, this, &DashboardPage::logoutRequested);
}

// destructor
DashboardPage::~DashboardPage()
{
    delete ui; // free the ui object
}

// updates the dashboard with user data
void DashboardPage::updateDashboard(int totalReceipts, double totalSpending, double avgMonthlySpending, QString topCategory, const QList<QPair<QString, double>> &spendingData)
{
    // update dashboard labels with formatted data
    ui->label_TotalReceipts->setText(QString("Total Receipts:\n\n%1").arg(totalReceipts));
    ui->label_TotalSpending->setText(QString("Total Spending:\n\n$%1").arg(totalSpending, 0, 'f', 2));
    ui->label_TopCategory->setText(QString("Top Category:\n\n%1").arg(topCategory));
    ui->label_AvgMonthlySpending->setText(QString("Avg Monthly Spending:\n\n$%1").arg(avgMonthlySpending, 0, 'f', 2));

    // create and populate a pie series for spending breakdown
    QPieSeries *series = new QPieSeries();
    for (const auto &data : spendingData) {
        series->append(data.first, data.second); // add category name and value
    }

    // create and configure a new chart
    QChart *chart = new QChart();
    chart->addSeries(series); // add the pie series to the chart
    chart->setTitle("Spending Breakdown");
    chart->setAnimationOptions(QChart::SeriesAnimations); // enable animations for better visuals

    // customize pie slices
    for (auto slice : series->slices()) {
        slice->setLabelVisible(true); // make labels visible
        slice->setPen(QPen(Qt::white)); // add white border to slices
        slice->setBrush(QBrush(slice->brush().color())); // retain slice color
    }

    // configure legend to show at the bottom
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    // set the configured chart to the chart view in the UI
    ui->spendingChartView->setChart(chart);
}
