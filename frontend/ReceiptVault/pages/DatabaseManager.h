#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase> // for managing database connection
#include <QString> // for string operations
#include <QList> // for storing lists
#include <QPair> // for storing key-value pairs
#include <QMap> // for mapping keys to values

class DatabaseManager
{
public:
    static DatabaseManager& instance(); // gets the singleton instance of the database manager

    bool openDatabase(const QString &databasePath); // opens the database and creates tables if not already present
    QSqlDatabase& getDatabase(); // returns the database object for queries

    // methods for managing users
    bool createUser(const QString &username, const QString &hashedPassword, const QString &salt); // creates a new user in the database
    bool verifyUser(const QString &username, const QString &hashedPassword); // verifies user credentials for login
    bool getUserCredentials(const QString &username, QString &hashedPassword, QString &salt); // gets hashed password and salt for a user

    // methods for managing categories
    bool addCategory(const QString &categoryName); // adds a new category to the database
    bool renameCategory(int categoryId, const QString &newName); // renames an existing category
    bool deleteCategory(int categoryId); // deletes a category by its id
    QList<QPair<int, QString>> getAllCategories(); // fetches all categories with their ids and names

    // methods for managing expenses
    bool addExpense(int userId, int categoryId, const QString &store, const QString &date, double amount, const QString &description); // adds a new expense
    bool deleteExpense(int expenseId, int userId); // deletes an expense by id, scoped to the owning user
    QList<QPair<QString, double>> getCategoryExpenses(int userId); // gets total expenses grouped by category for a user
    QList<QPair<QString, double>> getTopStores(int userId, int limit = 5); // gets top stores by spending for a user, limited to a specified number

    // methods for managing budgets
    double getSpendingForCategoryInPeriod(int userId, int categoryId, const QString &startDate, const QString &endDate); // calculates spending for a category within a date range

    // methods for dashboard data
    int getTotalReceipts(int userId); // gets the total number of receipts for a user
    double getTotalSpending(int userId); // gets the total spending amount for a user
    QString getTopSpendingCategory(int userId); // gets the top spending category for a user
    double getAverageMonthlySpending(int userId); // calculates average monthly spending for a user
    QList<QPair<QString, double>> getMonthlySpending(int userId); // gets spending grouped by month for a user

private:
    DatabaseManager(); // private constructor to enforce singleton pattern
    ~DatabaseManager(); // private destructor for cleanup

    DatabaseManager(const DatabaseManager&) = delete; // disables copying
    DatabaseManager& operator=(const DatabaseManager&) = delete; // disables assignment

    bool createTables(); // creates all required tables in the database
    QSqlDatabase db; // database object for connection management
};

#endif
