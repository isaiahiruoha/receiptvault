#ifndef CREATEACCOUNTPAGE_H
#define CREATEACCOUNTPAGE_H

#include <QWidget> // for QWidget base class

namespace Ui {
class CreateAccountPage; // forward declaration of ui class
}

class CreateAccountPage : public QWidget
{
    Q_OBJECT // needed for signals and slots

public:
    explicit CreateAccountPage(QWidget *parent = nullptr); // constructor to set up the widget
    ~CreateAccountPage(); // destructor to clean up the widget

signals:
    void accountCreationRequested(const QString &username, const QString &password); // signal for requesting account creation
    void navigateToLogin(); // signal to navigate back to the login page

private:
    Ui::CreateAccountPage *ui; // pointer to ui elements for this page
};

#endif
