#include "CreateAccountPage.h"
#include "ui_CreateAccountPage.h" // ui header for managing the ui design
#include <QMessageBox> // for showing message boxes

// constructor
CreateAccountPage::CreateAccountPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CreateAccountPage)
{
    ui->setupUi(this); // set up the UI from the .ui file

    // connect create account button to handle account creation
    connect(ui->createAccountButton, &QPushButton::clicked, [this]() {
        QString username = ui->createUsernameEdit->text().trimmed(); // get and trim username input
        QString password = ui->createPasswordEdit->text(); // get password input
        QString confirmPassword = ui->createConfirmPasswordEdit->text(); // get confirm password input

        // check if any field is empty
        if (username.isEmpty() || password.isEmpty() || confirmPassword.isEmpty()) {
            QMessageBox::warning(this, "Input Error", "Please fill in all fields.");
            return;
        }

        // check if passwords match
        if (password != confirmPassword) {
            QMessageBox::warning(this, "Input Error", "Passwords do not match.");
            return;
        }

        // emit signal to request account creation
        emit accountCreationRequested(username, password);
    });

    // connect back to login button to navigate to login page
    connect(ui->toLoginButton, &QPushButton::clicked, this, &CreateAccountPage::navigateToLogin);
}

// destructor
CreateAccountPage::~CreateAccountPage()
{
    delete ui; // clean up UI resources
}
