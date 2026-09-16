#ifndef RECEIPTSPAGE_H
#define RECEIPTSPAGE_H

#include <QWidget> // for QWidget base class
#include <QPushButton> // for buttons
#include <QTableWidget> // for table display
#include <QComboBox> // for dropdown menus

namespace Ui {
class ReceiptsPage; // forward declaration of ui class
}

class ReceiptsPage : public QWidget
{
    Q_OBJECT // needed for signals and slots

public:
    explicit ReceiptsPage(QWidget *parent = nullptr); // constructor to set up the widget
    ~ReceiptsPage(); // destructor to clean up resources

    void addReceipt(const QString &store, const QString &total, const QString &date, int categoryId, int expenseId); // adds a receipt to the table
    void loadReceipts(int userId); // loads receipts for the current user
    void setCurrentUserId(int userId); // sets the current user ID
    void populateCategoryComboBox(QComboBox *comboBox); // populates category dropdown with available categories

signals:
    void navigateToDashboard(); // signal to go back to the dashboard
    void uploadReceipt(); // signal to upload a new receipt
    void editReceipt(int expenseId); // signal to edit a receipt with the given expense ID

public slots:
    void editSelectedReceipt(); // handles editing the selected receipt
    void deleteSelectedReceipt(); // handles deleting the selected receipt

private:
    Ui::ReceiptsPage *ui; // pointer to ui elements for this page
    int currentUserId; // stores the id of the current user
};

#endif
