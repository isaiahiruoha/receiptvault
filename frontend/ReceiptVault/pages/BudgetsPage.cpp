#include "BudgetsPage.h"
#include "ui_BudgetsPage.h" // Include the generated UI header
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QComboBox>
#include <QLineEdit>
#include <QDateEdit>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QDebug>
#include <QBrush>
#include <QColor>
#include "DatabaseManager.h"

// Constructor
BudgetsPage::BudgetsPage(QWidget *parent) :
    QWidget(parent),
    currentUserId(-1),
    ui(new Ui::BudgetsPage)
{
    ui->setupUi(this); // Sets up the UI from the .ui file

    // Connect buttons to slots
    connect(ui->button_BackToDashboard, &QPushButton::clicked, this, &BudgetsPage::navigateToDashboard);
    connect(ui->button_AddBudget, &QPushButton::clicked, this, &BudgetsPage::addBudget);
    connect(ui->button_EditBudget, &QPushButton::clicked, this, &BudgetsPage::editBudget);
    connect(ui->button_DeleteBudget, &QPushButton::clicked, this, &BudgetsPage::deleteBudget);
}

// Destructor
BudgetsPage::~BudgetsPage()
{
    delete ui;
}

// loads budgets from the database for the user
void BudgetsPage::loadBudgets(int userId)
{
    currentUserId = userId;
    ui->table_Budgets->setRowCount(0); // Clear existing rows

    QSqlQuery query(DatabaseManager::instance().getDatabase());
    query.prepare(R"(
        SELECT
            budgets.budget_id,
            expense_category.category_id,
            expense_category.category_name,
            budgets.budget_amount,
            budgets.start_date,
            budgets.end_date
        FROM budgets
        JOIN expense_category ON budgets.category_id = expense_category.category_id
        WHERE budgets.user_id = :user_id
    )");
    query.bindValue(":user_id", userId);

    if (query.exec()) {
        while (query.next()) {
            int budgetId = query.value("budget_id").toInt();
            QString category = query.value("category_name").toString();
            double budgetAmount = query.value("budget_amount").toDouble();
            QString startDate = query.value("start_date").toString();
            QString endDate = query.value("end_date").toString();
            int categoryId = query.value("category_id").toInt();

            // grab total spending for this category and period
            double totalSpending = DatabaseManager::instance().getSpendingForCategoryInPeriod(userId, categoryId, startDate, endDate);

            // determine status
            QString statusText;
            QColor statusColor;

            double percentage = (totalSpending / budgetAmount) * 100.0;

            if (percentage <= 80.0) {
                statusText = "On Track";
                statusColor = QColor("#4CAF50"); // Green
            }
            else if (percentage <= 100.0) {
                statusText = "Caution";
                statusColor = QColor("#FFC107"); // Yellow
            }
            else {
                statusText = "Exceeded";
                statusColor = QColor("#F44336"); // Red
            }

            // Insert a new row
            int currentRow = ui->table_Budgets->rowCount();
            ui->table_Budgets->insertRow(currentRow);

            // Budget ID
            ui->table_Budgets->setItem(currentRow, 0, new QTableWidgetItem(QString::number(budgetId)));

            // Category
            ui->table_Budgets->setItem(currentRow, 1, new QTableWidgetItem(category));

            // Amount
            ui->table_Budgets->setItem(currentRow, 2, new QTableWidgetItem(QString::number(budgetAmount, 'f', 2)));

            // Start Date
            ui->table_Budgets->setItem(currentRow, 3, new QTableWidgetItem(startDate));

            // End Date
            ui->table_Budgets->setItem(currentRow, 4, new QTableWidgetItem(endDate));

            // Status
            QTableWidgetItem *statusItem = new QTableWidgetItem(statusText);
            statusItem->setTextAlignment(Qt::AlignCenter);
            statusItem->setForeground(QBrush(Qt::white));
            statusItem->setBackground(QBrush(statusColor));
            ui->table_Budgets->setItem(currentRow, 5, statusItem); // status is in the 6th column, index 5
        }
    } else {
        qDebug() << "Failed to load budgets:" << query.lastError().text();
        QMessageBox::critical(this, "Database Error", "Failed to load budgets from the database.");
    }
}

// Handles adding a new budget
void BudgetsPage::addBudget()
{
    // Dialog to input budget details
    QDialog dialog(this);
    dialog.setWindowTitle("Add Budget");
    dialog.setModal(true);

    QFormLayout formLayout(&dialog);

    // Category dropdown
    QComboBox *categoryComboBox = new QComboBox(&dialog);
    QSqlQuery categoryQuery(DatabaseManager::instance().getDatabase());
    categoryQuery.prepare("SELECT category_id, category_name FROM expense_category");
    if (categoryQuery.exec()) {
        while (categoryQuery.next()) {
            int categoryId = categoryQuery.value("category_id").toInt();
            QString categoryName = categoryQuery.value("category_name").toString();
            categoryComboBox->addItem(categoryName, categoryId);
        }
    } else {
        qDebug() << "Failed to load categories:" << categoryQuery.lastError().text();
        QMessageBox::critical(this, "Database Error", "Failed to load categories.");
        return;
    }
    formLayout.addRow("Category:", categoryComboBox);

    // Amount input
    QLineEdit *amountEdit = new QLineEdit(&dialog);
    amountEdit->setPlaceholderText("Enter budget amount");
    formLayout.addRow("Amount:", amountEdit);

    // Start Date input
    QDateEdit *startDateEdit = new QDateEdit(&dialog);
    startDateEdit->setCalendarPopup(true);
    startDateEdit->setDate(QDate::currentDate());
    formLayout.addRow("Start Date:", startDateEdit);

    // End Date input
    QDateEdit *endDateEdit = new QDateEdit(&dialog);
    endDateEdit->setCalendarPopup(true);
    endDateEdit->setDate(QDate::currentDate().addMonths(1));
    formLayout.addRow("End Date:", endDateEdit);

    // Dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    formLayout.addRow(buttonBox);

    // Connect dialog buttons
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    // Process dialog result
    if (dialog.exec() == QDialog::Accepted) {
        QString amountText = amountEdit->text().trimmed();
        if (amountText.isEmpty()) {
            QMessageBox::warning(this, "Input Error", "Enter a budget amount.");
            return;
        }

        bool ok;
        double amount = amountEdit->text().toDouble(&ok);
        if (!ok || amount <= 0) {
            QMessageBox::warning(this, "Input Error", "Enter a valid amount.");
            return;
        }

        int categoryId = categoryComboBox->currentData().toInt();
        QString startDate = startDateEdit->date().toString("yyyy-MM-dd");
        QString endDate = endDateEdit->date().toString("yyyy-MM-dd");

        // Add to database
        QSqlQuery insertQuery(DatabaseManager::instance().getDatabase());
        insertQuery.prepare(R"(
            INSERT INTO budgets (user_id, category_id, budget_amount, start_date, end_date)
            VALUES (:user_id, :category_id, :budget_amount, :start_date, :end_date)
        )");
        insertQuery.bindValue(":user_id", currentUserId);
        insertQuery.bindValue(":category_id", categoryId);
        insertQuery.bindValue(":budget_amount", amount);
        insertQuery.bindValue(":start_date", startDate);
        insertQuery.bindValue(":end_date", endDate);

        if (insertQuery.exec()) {
            QMessageBox::information(this, "Success", "Budget added successfully!");
            loadBudgets(currentUserId);
        } else {
            qDebug() << "Failed to add budget:" << insertQuery.lastError().text();
            QMessageBox::critical(this, "Database Error", "Failed to add budget.");
        }
    }
}

// Handles editing a budget
void BudgetsPage::editBudget()
{
    QList<QTableWidgetItem*> selectedItems = ui->table_Budgets->selectedItems();
    if (selectedItems.isEmpty()) {
        QMessageBox::warning(this, "Selection Error", "Select a budget to edit.");
        return;
    }

    int selectedRow = ui->table_Budgets->row(selectedItems.first());
    int budgetId = ui->table_Budgets->item(selectedRow, 0)->text().toInt();
    QString currentCategory = ui->table_Budgets->item(selectedRow, 1)->text();
    double currentAmount = ui->table_Budgets->item(selectedRow, 2)->text().toDouble();
    QString currentStartDate = ui->table_Budgets->item(selectedRow, 3)->text();
    QString currentEndDate = ui->table_Budgets->item(selectedRow, 4)->text();

    // Dialog to input new details
    QDialog dialog(this);
    dialog.setWindowTitle("Edit Budget");
    dialog.setModal(true);

    QFormLayout formLayout(&dialog);

    // Category dropdown
    QComboBox *categoryComboBox = new QComboBox(&dialog);
    QSqlQuery categoryQuery(DatabaseManager::instance().getDatabase());
    categoryQuery.prepare("SELECT category_id, category_name FROM expense_category");
    if (categoryQuery.exec()) {
        while (categoryQuery.next()) {
            int categoryId = categoryQuery.value("category_id").toInt();
            QString categoryName = categoryQuery.value("category_name").toString();
            categoryComboBox->addItem(categoryName, categoryId);
            if (categoryName == currentCategory) {
                categoryComboBox->setCurrentText(categoryName);
            }
        }
    } else {
        qDebug() << "Failed to load categories:" << categoryQuery.lastError().text();
        QMessageBox::critical(this, "Database Error", "Failed to load categories.");
        return;
    }
    formLayout.addRow("Category:", categoryComboBox);

    // Amount input
    QLineEdit *amountEdit = new QLineEdit(&dialog);
    amountEdit->setText(QString::number(currentAmount, 'f', 2));
    formLayout.addRow("Amount:", amountEdit);

    // Start Date input
    QDateEdit *startDateEdit = new QDateEdit(&dialog);
    startDateEdit->setCalendarPopup(true);
    startDateEdit->setDate(QDate::fromString(currentStartDate, "yyyy-MM-dd"));
    formLayout.addRow("Start Date:", startDateEdit);

    // End Date input
    QDateEdit *endDateEdit = new QDateEdit(&dialog);
    endDateEdit->setCalendarPopup(true);
    endDateEdit->setDate(QDate::fromString(currentEndDate, "yyyy-MM-dd"));
    formLayout.addRow("End Date:", endDateEdit);

    // Dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    formLayout.addRow(buttonBox);

    // Connect dialog buttons
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    // Process dialog result
    if (dialog.exec() == QDialog::Accepted) {
        QString amountText = amountEdit->text().trimmed();
        if (amountText.isEmpty()) {
            QMessageBox::warning(this, "Input Error", "Enter a budget amount.");
            return;
        }

        bool ok;
        double amount = amountEdit->text().toDouble(&ok);
        if (!ok || amount <= 0) {
            QMessageBox::warning(this, "Input Error", "Enter a valid amount.");
            return;
        }

        int categoryId = categoryComboBox->currentData().toInt();
        QString startDate = startDateEdit->date().toString("yyyy-MM-dd");
        QString endDate = endDateEdit->date().toString("yyyy-MM-dd");

        QSqlQuery updateQuery(DatabaseManager::instance().getDatabase());
        updateQuery.prepare(R"(
            UPDATE budgets
            SET category_id = :category_id,
                budget_amount = :budget_amount,
                start_date = :start_date,
                end_date = :end_date
            WHERE budget_id = :budget_id AND user_id = :user_id
        )");
        updateQuery.bindValue(":category_id", categoryId);
        updateQuery.bindValue(":budget_amount", amount);
        updateQuery.bindValue(":start_date", startDate);
        updateQuery.bindValue(":end_date", endDate);
        updateQuery.bindValue(":budget_id", budgetId);
        updateQuery.bindValue(":user_id", currentUserId);

        if (updateQuery.exec()) {
            QMessageBox::information(this, "Success", "Budget updated successfully!");
            loadBudgets(currentUserId);
        } else {
            qDebug() << "Failed to update budget:" << updateQuery.lastError().text();
            QMessageBox::critical(this, "Database Error", "Failed to update budget.");
        }
    }
}

// Handles deleting a budget
void BudgetsPage::deleteBudget()
{
    QList<QTableWidgetItem*> selectedItems = ui->table_Budgets->selectedItems();
    if (selectedItems.isEmpty()) {
        QMessageBox::warning(this, "Selection Error", "Select a budget to delete.");
        return;
    }

    int selectedRow = ui->table_Budgets->row(selectedItems.first());
    int budgetId = ui->table_Budgets->item(selectedRow, 0)->text().toInt();

    QMessageBox::StandardButton reply = QMessageBox::question(this, "Delete Budget","Are you sure you want to delete the selected budget?",QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        QSqlQuery deleteQuery(DatabaseManager::instance().getDatabase());
        deleteQuery.prepare(R"(
            DELETE FROM budgets
            WHERE budget_id = :budget_id AND user_id = :user_id
        )");
        deleteQuery.bindValue(":budget_id", budgetId);
        deleteQuery.bindValue(":user_id", currentUserId);

        if (deleteQuery.exec()) {
            QMessageBox::information(this, "Success", "Budget deleted successfully!");
            loadBudgets(currentUserId);
        } else {
            qDebug() << "Failed to delete budget:" << deleteQuery.lastError().text();
            QMessageBox::critical(this, "Database Error", "Failed to delete budget.");
        }
    }
}
