#include <QtTest>
#include <QSqlQuery>
#include "DatabaseManager.h"

class TestDatabaseManager : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void deleteCategory_clearsExpenseCategory();
    void deleteUser_cascadesExpenses();
    void addExpense_totalSpendingMatches();
    void getCategoryExpenses_showsUncategorizedAfterCategoryDelete();
    void expensesTable_hasIndexes();
    void deleteExpense_deniedForWrongUser();
};

void TestDatabaseManager::init()
{
    DatabaseManager::instance().getDatabase().close();
    QVERIFY(DatabaseManager::instance().openDatabase(":memory:"));
}

void TestDatabaseManager::cleanup()
{
    DatabaseManager::instance().getDatabase().close();
}

void TestDatabaseManager::deleteCategory_clearsExpenseCategory()
{
    auto &db = DatabaseManager::instance();
    db.createUser("alice", "hash", "salt");

    QSqlQuery q(db.getDatabase());
    q.exec("SELECT user_id FROM users WHERE username = 'alice'");
    q.next();
    int userId = q.value(0).toInt();

    q.exec("SELECT category_id FROM expense_category WHERE category_name = 'Groceries'");
    q.next();
    int categoryId = q.value(0).toInt();

    QVERIFY(db.addExpense(userId, categoryId, "Walmart", "2026-01-01", 42.50, "test"));
    QVERIFY(db.deleteCategory(categoryId));

    q.exec("SELECT category_id FROM expenses WHERE store = 'Walmart'");
    q.next();
    QVERIFY(q.value(0).isNull()); // ON DELETE SET NULL
}

void TestDatabaseManager::deleteUser_cascadesExpenses()
{
    auto &db = DatabaseManager::instance();
    db.createUser("bob", "hash", "salt");

    QSqlQuery q(db.getDatabase());
    q.exec("SELECT user_id FROM users WHERE username = 'bob'");
    q.next();
    int userId = q.value(0).toInt();

    q.exec("SELECT category_id FROM expense_category LIMIT 1");
    q.next();
    int categoryId = q.value(0).toInt();

    db.addExpense(userId, categoryId, "Shell", "2026-01-01", 30.00, "gas");

    q.prepare("DELETE FROM users WHERE user_id = :id");
    q.bindValue(":id", userId);
    QVERIFY(q.exec());

    q.exec(QString("SELECT COUNT(*) FROM expenses WHERE user_id = %1").arg(userId));
    q.next();
    QCOMPARE(q.value(0).toInt(), 0); // ON DELETE CASCADE
}

void TestDatabaseManager::addExpense_totalSpendingMatches()
{
    auto &db = DatabaseManager::instance();
    db.createUser("carol", "hash", "salt");

    QSqlQuery q(db.getDatabase());
    q.exec("SELECT user_id FROM users WHERE username = 'carol'");
    q.next();
    int userId = q.value(0).toInt();

    q.exec("SELECT category_id FROM expense_category LIMIT 1");
    q.next();
    int categoryId = q.value(0).toInt();

    db.addExpense(userId, categoryId, "Costco", "2026-01-01", 100.00, "");
    db.addExpense(userId, categoryId, "Costco", "2026-01-02", 50.00, "");

    QCOMPARE(db.getTotalSpending(userId), 150.00);
}

void TestDatabaseManager::getCategoryExpenses_showsUncategorizedAfterCategoryDelete()
{
    auto &db = DatabaseManager::instance();
    db.createUser("frank", "hash", "salt");

    QSqlQuery q(db.getDatabase());
    q.exec("SELECT user_id FROM users WHERE username = 'frank'");
    q.next();
    int userId = q.value(0).toInt();

    q.exec("SELECT category_id FROM expense_category WHERE category_name = 'Health'");
    q.next();
    int categoryId = q.value(0).toInt();

    db.addExpense(userId, categoryId, "CVS", "2026-01-01", 25.00, "");
    db.deleteCategory(categoryId);

    bool found = false;
    for (const auto &pair : db.getCategoryExpenses(userId)) {
        if (pair.first == "Uncategorized") {
            QCOMPARE(pair.second, 25.00);
            found = true;
        }
    }
    QVERIFY(found);
}

void TestDatabaseManager::expensesTable_hasIndexes()
{
    QSqlQuery q(DatabaseManager::instance().getDatabase());
    q.exec("PRAGMA index_list('expenses')");
    int count = 0;
    while (q.next()) count++;
    QVERIFY(count >= 2);
}

void TestDatabaseManager::deleteExpense_deniedForWrongUser()
{
    auto &db = DatabaseManager::instance();
    db.createUser("dave", "hash", "salt");
    db.createUser("eve", "hash", "salt");

    QSqlQuery q(db.getDatabase());
    q.exec("SELECT user_id FROM users WHERE username = 'eve'");
    q.next();
    int eveId = q.value(0).toInt();

    q.exec("SELECT user_id FROM users WHERE username = 'dave'");
    q.next();
    int daveId = q.value(0).toInt();

    q.exec("SELECT category_id FROM expense_category LIMIT 1");
    q.next();
    int categoryId = q.value(0).toInt();

    db.addExpense(daveId, categoryId, "Target", "2026-01-01", 20.00, "");

    q.exec("SELECT expense_id FROM expenses WHERE store = 'Target'");
    q.next();
    int expenseId = q.value(0).toInt();

    db.deleteExpense(expenseId, eveId); // wrong user - exec() still succeeds, but 0 rows match

    q.exec(QString("SELECT COUNT(*) FROM expenses WHERE expense_id = %1").arg(expenseId));
    q.next();
    QCOMPARE(q.value(0).toInt(), 1); // still exists
}

QTEST_MAIN(TestDatabaseManager)
#include "tst_databasemanager.moc"
