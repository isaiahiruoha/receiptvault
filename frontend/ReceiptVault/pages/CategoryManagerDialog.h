#ifndef CATEGORYMANAGERDIALOG_H
#define CATEGORYMANAGERDIALOG_H

#include <QDialog> // for dialog window
#include <QListWidget> // for showing list of categories
#include <QPushButton> // for buttons
#include <QVBoxLayout> // for vertical layout
#include <QHBoxLayout> // for horizontal layout
#include <QInputDialog> // for input dialog
#include <QMessageBox> // for message boxes

class CategoryManagerDialog : public QDialog
{
    Q_OBJECT // needed for signals and slots

public:
    explicit CategoryManagerDialog(QWidget *parent = nullptr); // constructor for setting up dialog

private slots:
    void addCategory(); // adds a new category
    void renameCategory(); // renames an existing category
    void deleteCategory(); // deletes a category

private:
    QListWidget *listWidgetCategories; // widget to display list of categories
    QPushButton *buttonAdd; // button to add category
    QPushButton *buttonRename; // button to rename category
    QPushButton *buttonDelete; // button to delete category
    QPushButton *buttonClose; // button to close the dialog

    void loadCategories(); // loads categories from database
};

#endif
