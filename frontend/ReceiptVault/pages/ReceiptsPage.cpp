#include "ReceiptsPage.h"
#include "pages/CategoryManagerDialog.h" // Include category manager dialog
#include "ui_ReceiptsPage.h" // Include the generated UI header
#include <QDebug>
#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QComboBox>
#include <QMessageBox>
#include <QDialog>

// constructor
ReceiptsPage::ReceiptsPage(QWidget *parent)
    : QWidget(parent),
    ui(new Ui::ReceiptsPage),
    currentUserId(-1) // Initialize user ID
{
    ui->setupUi(this); // Sets up the UI from the .ui file

    // connect buttons to their respective slots
    connect(ui->button_BackToDashboard, &QPushButton::clicked, this, &ReceiptsPage::navigateToDashboard);
    connect(ui->button_UploadReceipt, &QPushButton::clicked, this, &ReceiptsPage::uploadReceipt);
    connect(ui->button_EditReceipt, &QPushButton::clicked, this, &ReceiptsPage::editSelectedReceipt);

    // connect the Edit Categories button to open the category manager dialog
    connect(ui->button_EditCategories, &QPushButton::clicked, this, [this]() {
        CategoryManagerDialog dialog(this);
        if (dialog.exec() == QDialog::Accepted) {
            // refresh receipts after category changes
            loadReceipts(currentUserId);
        }
    });
}

// destructor
ReceiptsPage::~ReceiptsPage()
{
    delete ui; // Clean up UI resources
}

// populates the category combo box with available categories
void ReceiptsPage::populateCategoryComboBox(QComboBox *comboBox)
{
    QList<QPair<int, QString>> categories = DatabaseManager::instance().getAllCategories();
    for (const QPair<int, QString> &category : categories) {
        comboBox->addItem(category.second, category.first); // Add category name and ID
    }
}

// adds a new receipt to the table
void ReceiptsPage::addReceipt(const QString &store, const QString &total, const QString &date, int categoryId, int expenseId)
{
    int currentRow = ui->table_Receipts->rowCount();
    ui->table_Receipts->insertRow(currentRow); // Add a new row

    // store column
    QTableWidgetItem *storeItem = new QTableWidgetItem(store);
    storeItem->setData(Qt::UserRole, expenseId); // Store expense ID in UserRole
    ui->table_Receipts->setItem(currentRow, 0, storeItem);

    // total column
    QTableWidgetItem *totalItem = new QTableWidgetItem(total);
    ui->table_Receipts->setItem(currentRow, 1, totalItem);

    // date column
    QTableWidgetItem *dateItem = new QTableWidgetItem(date);
    ui->table_Receipts->setItem(currentRow, 2, dateItem);

    // category dropdown
    QComboBox *categoryComboBox = new QComboBox(this);
    populateCategoryComboBox(categoryComboBox); // Fill dropdown with categories

    // set current category in the dropdown
    int comboBoxIndex = categoryComboBox->findData(categoryId);
    if (comboBoxIndex != -1) {
        categoryComboBox->setCurrentIndex(comboBoxIndex);
    }

    // connect category changes to update the database
    connect(categoryComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
        QVariant data = categoryComboBox->itemData(index);
        if (data.isValid()) {
            int selectedCategoryId = data.toInt();
            int expenseId = storeItem->data(Qt::UserRole).toInt();

            // update the database with the new category
            QSqlQuery updateQuery(DatabaseManager::instance().getDatabase());
            updateQuery.prepare("UPDATE expenses SET category_id = :category_id WHERE expense_id = :expense_id AND user_id = :user_id");
            updateQuery.bindValue(":category_id", selectedCategoryId);
            updateQuery.bindValue(":expense_id", expenseId);
            updateQuery.bindValue(":user_id", currentUserId);

            if (!updateQuery.exec()) {
                qDebug() << "Failed to update category:" << updateQuery.lastError().text();
                QMessageBox::critical(this, "Database Error", "Failed to update receipt category.");
            }
        }
    });

    ui->table_Receipts->setCellWidget(currentRow, 3, categoryComboBox); // Add dropdown to the table
}

// loads receipts from the database for the current user
void ReceiptsPage::loadReceipts(int userId)
{
    ui->table_Receipts->setRowCount(0); // Clear existing rows
    currentUserId = userId; // Store user ID

    QSqlQuery query(DatabaseManager::instance().getDatabase());
    query.prepare("SELECT expense_id, store, expense_amount, expense_date, category_id FROM expenses WHERE user_id = :user_id");
    query.bindValue(":user_id", userId);

    if (query.exec()) {
        while (query.next()) {
            int expenseId = query.value("expense_id").toInt();
            QString store = query.value("store").toString();
            double amount = query.value("expense_amount").toDouble();
            QString total = QString::number(amount, 'f', 2);
            QString date = query.value("expense_date").toString();
            int categoryId = query.value("category_id").toInt();

            addReceipt(store, total, date, categoryId, expenseId); // Add receipt to the table
        }
    } else {
        qDebug() << "Error loading receipts:" << query.lastError().text();
        QMessageBox::critical(this, "Database Error", "Failed to load receipts from the database.");
    }
}

// sets the current user ID
void ReceiptsPage::setCurrentUserId(int userId)
{
    currentUserId = userId;
}

// handles editing the selected receipt
void ReceiptsPage::editSelectedReceipt()
{
    QList<QTableWidgetItem*> selectedItems = ui->table_Receipts->selectedItems();
    if (selectedItems.isEmpty()) {
        QMessageBox::warning(this, "Selection Error", "Please select a receipt to edit.");
        return;
    }

    int selectedRow = ui->table_Receipts->row(selectedItems.first());

    // retrieve the expense ID from UserRole
    QTableWidgetItem *storeItem = ui->table_Receipts->item(selectedRow, 0);
    int expenseId = storeItem->data(Qt::UserRole).toInt();

    // emit signal for editing the selected receipt
    emit editReceipt(expenseId);
}
