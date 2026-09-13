#ifndef LOGINPAGE_H
#define LOGINPAGE_H

#include <QWidget> // for QWidget base class

namespace Ui {
class LoginPage; // forward declaration of ui class
}

class LoginPage : public QWidget
{
    Q_OBJECT // needed for signals and slots

public:
    explicit LoginPage(QWidget *parent = nullptr); // constructor to set up the widget
    ~LoginPage(); // destructor to clean up resources

    void clearFields(); // clears the username/password fields (e.g. on logout)

signals:
    void loginRequested(const QString &username, const QString &password); // signal to request login with username and password
    void navigateToCreateAccount(); // signal to navigate to the create account page

private:
    Ui::LoginPage *ui; // pointer to ui elements for this page
};

#endif
