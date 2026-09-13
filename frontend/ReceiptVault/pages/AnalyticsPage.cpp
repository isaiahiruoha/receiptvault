#include "AnalyticsPage.h"
#include "ui_AnalyticsPage.h"
#include <QtCharts/QPieSeries> // for pie chart
#include <QtCharts/QLineSeries> // for line chart
#include <QtCharts/QBarSeries> // for bar chart
#include <QtCharts/QBarSet> // for bar sets
#include <QtCharts/QChart> // for managing charts
#include <QtCharts/QBarCategoryAxis> // for bar chart x-axis
#include <QDebug> // for debugging
#include <QtCharts/QValueAxis> // for value axis

AnalyticsPage::AnalyticsPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AnalyticsPage)
{
    ui->setupUi(this);

    // initialize spending breakdown pie chart
    updateSpendingBreakdown(QList<QPair<QString, double>>());

    // initialize monthly spending trend line chart
    updateMonthlyTrend(QList<QPair<QString, double>>());

    // initialize category-wise expense comparison bar chart
    updateCategoryComparison(QList<QPair<QString, double>>());

    // initialize top stores by spending bar chart
    updateTopStores(QList<QPair<QString, double>>());

    // connect back button to navigate to dashboard signal
    connect(ui->button_BackToDashboard, &QPushButton::clicked, this, &AnalyticsPage::navigateToDashboard);
}

AnalyticsPage::~AnalyticsPage()
{
    delete ui; // clean up ui
}

void AnalyticsPage::updateChartData(
    const QList<QPair<QString, double>> &categoryExpenses,
    const QList<QPair<QString, double>> &monthlySpending,
    const QList<QPair<QString, double>> &topStores
    )
{
    updateSpendingBreakdown(categoryExpenses); // update pie chart
    updateMonthlyTrend(monthlySpending); // update line chart
    updateCategoryComparison(categoryExpenses); // update bar chart
    updateTopStores(topStores); // update top stores chart
}

void AnalyticsPage::updateSpendingBreakdown(const QList<QPair<QString, double>> &data)
{
    if (!ui->chartPlaceholder || !ui->chartPlaceholder->chart()) {
        qDebug() << "chart view or chart is not initialized";
        return;
    }

    QChart *chart = ui->chartPlaceholder->chart();

    // clear existing data
    chart->removeAllSeries();

    // create new pie series
    QPieSeries *pieSeries = new QPieSeries();
    pieSeries->setName("Spending Breakdown");
    for (const auto &pair : data) {
        pieSeries->append(pair.first, pair.second);
    }

    // update chart with new series
    chart->addSeries(pieSeries);
    chart->setTitle("Spending Breakdown");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    // customize pie slices
    for (auto slice : pieSeries->slices()) {
        slice->setLabelVisible(true);
        slice->setPen(QPen(Qt::white));
        slice->setBrush(QBrush(slice->brush().color()));
    }

    // adjust legend
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
}

void AnalyticsPage::updateMonthlyTrend(const QList<QPair<QString, double>> &data)
{
    if (!ui->chartView_MonthlyTrend || !ui->chartView_MonthlyTrend->chart()) {
        qDebug() << "monthly trend chart view or chart is not initialized";
        return;
    }

    QChart *chart = ui->chartView_MonthlyTrend->chart();

    // Clear existing data
    chart->removeAllSeries();

    // **Remove existing axes**
    QList<QAbstractAxis*> axes = chart->axes();
    for (QAbstractAxis *axis : axes) {
        chart->removeAxis(axis);
        delete axis; // Delete the axis to prevent memory leaks
    }

    // Create new line series
    QLineSeries *lineSeries = new QLineSeries();
    lineSeries->setName("Monthly Spending");

    QStringList categories;
    for (const auto &pair : data) {
        categories.append(pair.first); // Add category
        lineSeries->append(categories.size(), pair.second); // x-axis index
    }

    // Configure chart
    chart->addSeries(lineSeries);
    chart->setTitle("Monthly Spending Trend");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    // Create axes
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    lineSeries->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Amount ($)");
    chart->addAxis(axisY, Qt::AlignLeft);
    lineSeries->attachAxis(axisY);

    // Customize line
    lineSeries->setPointsVisible(true);
    lineSeries->setPen(QPen(Qt::blue, 2));

    // Adjust legend
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
}

void AnalyticsPage::updateCategoryComparison(const QList<QPair<QString, double>> &data)
{
    if (!ui->chartView_CategoryComparison || !ui->chartView_CategoryComparison->chart()) {
        qDebug() << "category comparison chart view or chart is not initialized";
        return;
    }

    QChart *chart = ui->chartView_CategoryComparison->chart();

    // Clear existing data
    chart->removeAllSeries();

    // clear axis
    QList<QAbstractAxis*> axes = chart->axes();
    for (QAbstractAxis *axis : axes) {
        chart->removeAxis(axis);
        delete axis; // Delete the axis to prevent memory leaks
    }

    // create new bar series
    QBarSeries *barSeries = new QBarSeries();
    barSeries->setName("Category Expenses");

    QBarSet *barSet = new QBarSet("Expenses");
    for (const auto &pair : data) {
        *barSet << pair.second; // add values
    }

    barSeries->append(barSet);
    chart->addSeries(barSeries);
    chart->setTitle("Category-wise Expense Comparison");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    // create axes
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    QStringList categories;
    for (const auto &pair : data) {
        categories.append(pair.first); // add category names
    }
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    barSeries->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Amount ($)");
    chart->addAxis(axisY, Qt::AlignLeft);
    barSeries->attachAxis(axisY);

    // customize bar colors
    barSet->setColor(QColor("#42A5F5")); // blue color

    // adjust legend
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
}

void AnalyticsPage::updateTopStores(const QList<QPair<QString, double>> &data)
{
    if (!ui->chartView_TopStores || !ui->chartView_TopStores->chart()) {
        qDebug() << "top stores chart view or chart is not initialized";
        return;
    }

    QChart *chart = ui->chartView_TopStores->chart();

    // Clear existing data
    chart->removeAllSeries();

    // remove axis
    QList<QAbstractAxis*> axes = chart->axes();
    for (QAbstractAxis *axis : axes) {
        chart->removeAxis(axis);
        delete axis; // Delete the axis to prevent memory leaks
    }

    // create new bar series
    QBarSeries *barSeries = new QBarSeries();
    barSeries->setName("Top Stores");

    QBarSet *barSet = new QBarSet("Spending");
    for (const auto &pair : data) {
        *barSet << pair.second; // add values
    }

    barSeries->append(barSet);
    chart->addSeries(barSeries);
    chart->setTitle("Top Stores by Spending");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    // create axes
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    QStringList stores;
    for (const auto &pair : data) {
        stores.append(pair.first); // add store names
    }
    axisX->append(stores);
    chart->addAxis(axisX, Qt::AlignBottom);
    barSeries->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Amount ($)");
    chart->addAxis(axisY, Qt::AlignLeft);
    barSeries->attachAxis(axisY);

    // customize bar colors
    barSet->setColor(QColor("#66BB6A")); // green color

    // adjust legend
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
}
