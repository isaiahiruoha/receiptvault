#include "CategoryManagerDialog.h"
#include "DatabaseManager.h" // for database operations

// constructor for category manager dialog
CategoryManagerDialog::CategoryManagerDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Manage Categories"); // set dialog title
    setMinimumSize(400, 300); // set minimum size for dialog

    QVBoxLayout *mainLayout = new QVBoxLayout(this); // main layout for the dialog

    // list widget to display existing categories
    listWidgetCategories = new QListWidget(this);
    loadCategories(); // load categories from the database
    mainLayout->addWidget(listWidgetCategories);

    // horizontal layout for buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    // create buttons for add, rename, delete, and close actions
    buttonAdd = new QPushButton("Add", this);
    buttonRename = new QPushButton("Rename", this);
    buttonDelete = new QPushButton("Delete", this);
    buttonClose = new QPushButton("Close", this);

    // add buttons to the layout
    buttonLayout->addWidget(buttonAdd);
    buttonLayout->addWidget(buttonRename);
    buttonLayout->addWidget(buttonDelete);
    buttonLayout->addStretch(); // push the close button to the right
    buttonLayout->addWidget(buttonClose);

    mainLayout->addLayout(buttonLayout); // add button layout to the main layout

    // connect button clicks to their respective slots
    connect(buttonAdd, &QPushButton::clicked, this, &CategoryManagerDialog::addCategory);
    connect(buttonRename, &QPushButton::clicked, this, &CategoryManagerDialog::renameCategory);
    connect(buttonDelete, &QPushButton::clicked, this, &CategoryManagerDialog::deleteCategory);
    connect(buttonClose, &QPushButton::clicked, this, &CategoryManagerDialog::accept);

    setObjectName("CategoryManagerDialog"); // set object name for debugging or styling
}

// loads categories from the database into the list widget
void CategoryManagerDialog::loadCategories()
{
    listWidgetCategories->clear(); // clear the list widget
    QList<QPair<int, QString>> categories = DatabaseManager::instance().getAllCategories(); // fetch categories

    // add each category to the list widget
    for (const QPair<int, QString> &category : categories) {
        QListWidgetItem *item = new QListWidgetItem(category.second, listWidgetCategories);
        item->setData(Qt::UserRole, category.first); // store category_id in item data
    }
}

// adds a new category
void CategoryManagerDialog::addCategory()
{
    bool ok;
    QString categoryName = QInputDialog::getText(this, "Add Category", "Category Name:", QLineEdit::Normal, "", &ok);

    if (ok && !categoryName.trimmed().isEmpty()) {
        if (DatabaseManager::instance().addCategory(categoryName.trimmed())) {
            QMessageBox::information(this, "Success", "Category added successfully.");
            loadCategories(); // refresh categories list
        } else {
            QMessageBox::critical(this, "Error", "Failed to add category. It might already exist.");
        }
    }
}

// renames the selected category
void CategoryManagerDialog::renameCategory()
{
    QListWidgetItem *selectedItem = listWidgetCategories->currentItem(); // get selected item
    if (!selectedItem) {
        QMessageBox::warning(this, "Selection Error", "Please select a category to rename.");
        return;
    }

    int categoryId = selectedItem->data(Qt::UserRole).toInt(); // get category_id
    QString currentName = selectedItem->text(); // get current category name

    bool ok;
    QString newName = QInputDialog::getText(this, "Rename Category", "New Category Name:", QLineEdit::Normal, currentName, &ok);

    if (ok && !newName.trimmed().isEmpty()) {
        if (DatabaseManager::instance().renameCategory(categoryId, newName.trimmed())) {
            QMessageBox::information(this, "Success", "Category renamed successfully.");
            loadCategories(); // refresh categories list
        } else {
            QMessageBox::critical(this, "Error", "Failed to rename category. The new name might already exist.");
        }
    }
}

// deletes the selected category
void CategoryManagerDialog::deleteCategory()
{
    QListWidgetItem *selectedItem = listWidgetCategories->currentItem(); // get selected item
    if (!selectedItem) {
        QMessageBox::warning(this, "Selection Error", "Please select a category to delete.");
        return;
    }

    int categoryId = selectedItem->data(Qt::UserRole).toInt(); // get category_id
    QString categoryName = selectedItem->text(); // get category name

    // confirm deletion
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Delete Category",
        QString("Are you sure you want to delete the category '%1'? Any associated receipts will have their category cleared, not deleted.")
            .arg(categoryName),
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        if (DatabaseManager::instance().deleteCategory(categoryId)) {
            QMessageBox::information(this, "Success", "Category deleted successfully.");
            loadCategories(); // refresh categories list
        } else {
            QMessageBox::critical(this, "Error", "Failed to delete category.");
        }
    }
}
