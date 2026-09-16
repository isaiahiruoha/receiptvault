#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow> // for main application window
#include <QStackedWidget> // for managing multiple pages
#include "pages/LoginPage.h" // login page header
#include "pages/CreateAccountPage.h" // create account page header
#include "pages/DashboardPage.h" // dashboard page header
#include "pages/ReceiptsPage.h" // receipts page header
#include "pages/AnalyticsPage.h" // analytics page header
#include "pages/BudgetsPage.h" // budgets page header
#include <QMap> // for mapping widgets to their styles
#include <QWidget> // for general widgets

class MainWindow : public QMainWindow
{
    Q_OBJECT // needed for signals and slots

public:
    explicit MainWindow(QWidget *parent = nullptr); // constructor to set up the main window
    ~MainWindow(); // destructor to clean up resources

private slots:
    void handleLogin(const QString &username, const QString &password); // handles login functionality
    void handleCreateAccount(const QString &username, const QString &password); // handles account creation
    void navigateToDashboard(); // navigates to the dashboard page
    void navigateToLogin(); // navigates back to the login page
    void handleUploadReceipt(); // handles receipt upload functionality
    void handleNavigateToAnalytics(); // navigates to the analytics page
    void handleEditReceipt(int expenseId); // handles editing of a specific receipt

private:
    QStackedWidget *stackedWidget; // stack to manage switching between pages

    // page instances
    LoginPage *loginPage; // login page instance
    CreateAccountPage *createAccountPage; // create account page instance
    DashboardPage *dashboardPage; // dashboard page instance
    ReceiptsPage *receiptsPage; // receipts page instance
    AnalyticsPage *analyticsPage; // analytics page instance
    BudgetsPage *budgetsPage; // budgets page instance

    void applyStyles(bool darkMode); // applies external styles based on theme
    void clearInlineStyles(QWidget* widget); // clears inline styles for dark mode
    void restoreInlineStyles(QWidget* widget); // restores inline styles for light mode
    QMap<QWidget*, QString> originalStyles; // map to store original widget styles for light mode

    int getCurrentUserId(); // gets the current user ID
    QString normalizeReceiptDate(QString date); // reformats an extracted receipt date to yyyy-MM-dd, or "" if unparseable

    QString currentUsername; // stores the current user's username
    int currentUserId; // stores the current user's ID

    // theme management
    bool toggleDarkMode; // keeps track of current theme mode
    bool isUpdatingTheme; // prevents recursive updates when switching themes
};

#endif
