#include "LoginPage.h"
#include "ui_LoginPage.h" // ui header for managing the ui design
#include <QMessageBox> // For showing warnings

// constructor
LoginPage::LoginPage(QWidget *parent)
    : QWidget(parent), ui(new Ui::LoginPage) // initialize the UI pointer
{
    ui->setupUi(this); // set up the UI from the .ui file

    // connect the login button to handle login requests
    connect(ui->loginButton, &QPushButton::clicked, this, [this]() {
        QString username = ui->loginUsernameEdit->text().trimmed(); // get and trim username input
        QString password = ui->loginPasswordEdit->text(); // get password input

        // check if username or password is empty
        if (username.isEmpty() || password.isEmpty()) {
            QMessageBox::warning(this, "Input Error", "Please enter both username and password.");
            return; // exit if inputs are invalid
        }

        emit loginRequested(username, password); // emit signal for login (handled in main window)
    });

    // connect the create account button to navigate to the create account page
    connect(ui->toCreateAccountButton, &QPushButton::clicked, this, &LoginPage::navigateToCreateAccount);
}

// destructor
LoginPage::~LoginPage()
{
    delete ui; // clean up the UI object
}

// clears the username/password fields (e.g. after logout)
void LoginPage::clearFields()
{
    ui->loginUsernameEdit->clear();
    ui->loginPasswordEdit->clear();
}
