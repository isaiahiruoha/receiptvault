#ifndef BUDGETSPAGE_H
#define BUDGETSPAGE_H

#include <QWidget>
#include <QPushButton> // for buttons
#include <QTableWidget> // for table display
#include <QLineEdit> // for input fields
#include <QComboBox> // for dropdown menus
#include <QDateEdit> // for date selection
#include <QFormLayout> // for form layouts
#include <QVBoxLayout> // for vertical layouts
#include <QHBoxLayout> // for horizontal layouts

namespace Ui {
class BudgetsPage; // forward declaration of ui class
}

class BudgetsPage : public QWidget
{
    Q_OBJECT // needed for signals and slots

public:
    explicit BudgetsPage(QWidget *parent = nullptr); // constructor to set up the widget
    ~BudgetsPage(); // destructor to clean up the widget

    void loadBudgets(int userId); // loads budgets for a specific user

signals:
    void navigateToDashboard(); // signal to go back to the dashboard

private slots:
    void addBudget(); // adds a new budget
    void editBudget(); // edits an existing budget
    void deleteBudget(); // deletes a budget

private:
    int currentUserId; // stores the id of the current user
    Ui::BudgetsPage *ui; // pointer to ui elements
};

#endif
