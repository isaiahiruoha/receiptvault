#include "mainwindow.h"
#include <QSqlError>
#include <QSqlQuery>
#include <QMessageBox>
#include <QFile>
#include <QDebug>
#include <QFileDialog>
#include <QtCharts/QPieSeries>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include "pages/DatabaseManager.h"
#include <QCryptographicHash>
#include <QInputDialog>
#include <QLineEdit>
#include <QProcess>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMenuBar>
#include <QMenu>
#include <QStandardPaths>
#include <QDir>
#include <QApplication>

// constructor for MainWindow
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), isUpdatingTheme(false)
{


    // Resolve a stable, writable location for the database so it works whether
    // the app is run from Qt Creator or launched as a bundled .app (a relative
    // path here depends on the launch working directory, which is why it broke).
    // The schema is created on first run by DatabaseManager::createTables().
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    QString dbPath = dataDir + "/receiptvault.db";

    // use DatabaseManager to open the database
    if (!DatabaseManager::instance().openDatabase(dbPath)) {
        QMessageBox::critical(this, "Database Connection Error",
                              "Unable to connect to the database at:\n" + dbPath);
        exit(EXIT_FAILURE);
    }

    // set the window title and size
    setWindowTitle("ReceiptVault - Budget Tracking App");
    resize(1000, 600);

    // create and set the stacked widget
    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);

    // initialize all the pages
    loginPage = new LoginPage(this);
    createAccountPage = new CreateAccountPage(this);
    dashboardPage = new DashboardPage(this);
    receiptsPage = new ReceiptsPage(this);
    analyticsPage = new AnalyticsPage(this);
    budgetsPage = new BudgetsPage(this);

    // create a QMenuBar and a QMenu for Dark Mode
    QMenuBar* menuBar = new QMenuBar();
    QMenu* settingsMenu = menuBar->addMenu("Settings");

    // create a checkbox action for Dark Mode and initialize to light
    QAction* darkModeAction = new QAction("Dark Mode");
    darkModeAction->setCheckable(true);
    darkModeAction->setChecked(false);  // default is light mode
    toggleDarkMode = false;

    // connect the action to toggle function
    // Inside MainWindow constructor after creating darkModeAction
    connect(darkModeAction, &QAction::toggled, [this](bool checked) {
        if (isUpdatingTheme) return; // Prevent recursive updates

        toggleDarkMode = checked;
        isUpdatingTheme = true;

        if (toggleDarkMode) {
            clearInlineStyles(this); // clear inline styles for dark mode
            applyStyles(true);       // apply dark mode stylesheet
        } else {
            restoreInlineStyles(this); // restore original styles for light mode
            applyStyles(false);        // apply light mode stylesheet
        }

        isUpdatingTheme = false; // Reset the flag after applying styles
    });


    // add the action to the settings menu
    settingsMenu->addAction(darkModeAction);

    // set the menu bar for the window
    setMenuBar(menuBar);


    // add all the pages to the stacked widget
    stackedWidget->addWidget(loginPage);
    stackedWidget->addWidget(createAccountPage);
    stackedWidget->addWidget(dashboardPage);
    stackedWidget->addWidget(receiptsPage);
    stackedWidget->addWidget(analyticsPage);
    stackedWidget->addWidget(budgetsPage);

    // display the login page first
    stackedWidget->setCurrentWidget(loginPage);

    applyStyles(toggleDarkMode);

    // navigate to CreateAccountPage
    connect(loginPage, &LoginPage::navigateToCreateAccount, [this]() {
        stackedWidget->setCurrentWidget(createAccountPage);
    });

    // navigate back to LoginPage
    connect(createAccountPage, &CreateAccountPage::navigateToLogin, [this]() {
        stackedWidget->setCurrentWidget(loginPage);
    });

    // connect signals for LoginPage
    connect(loginPage, &LoginPage::loginRequested, this, &MainWindow::handleLogin);

    // connect signals for CreateAccountPage
    connect(createAccountPage, &CreateAccountPage::accountCreationRequested, this, &MainWindow::handleCreateAccount);

    // connect signals for DashboardPage
    connect(dashboardPage, &DashboardPage::navigateToReceipts, [this]() {
        if (currentUserId != -1) {
            receiptsPage->loadReceipts(currentUserId);
            stackedWidget->setCurrentWidget(receiptsPage);
        } else {
            QMessageBox::warning(this, "Error", "User ID not found. Please log in again.");
        }
    });

    connect(dashboardPage, &DashboardPage::navigateToAnalytics, this, &MainWindow::handleNavigateToAnalytics);

    connect(dashboardPage, &DashboardPage::navigateToBudgets, [this]() {
        if (currentUserId != -1) {
            budgetsPage->loadBudgets(currentUserId);
            stackedWidget->setCurrentWidget(budgetsPage);
        } else {
            QMessageBox::warning(this, "Error", "User ID not found. Please log in again.");
        }
    });

    connect(dashboardPage, &DashboardPage::logoutRequested, [this]() {
        // clear any sensitive information
        loginPage->clearFields();
        currentUsername.clear();
        currentUserId = -1; // reset currentUserId
        // navigate back to the login page
        stackedWidget->setCurrentWidget(loginPage);
    });

    // connect signals for ReceiptsPage
    connect(receiptsPage, &ReceiptsPage::navigateToDashboard, this, &MainWindow::navigateToDashboard);
    connect(receiptsPage, &ReceiptsPage::uploadReceipt, this, &MainWindow::handleUploadReceipt);

    // connect signals for AnalyticsPage and BudgetsPage
    connect(analyticsPage, &AnalyticsPage::navigateToDashboard, this, &MainWindow::navigateToDashboard);
    connect(budgetsPage, &BudgetsPage::navigateToDashboard, this, &MainWindow::navigateToDashboard);

    connect(receiptsPage, &ReceiptsPage::editReceipt, this, &MainWindow::handleEditReceipt);

    this->setStyleSheet("background-color: #FFFFFF; color: #000000;");
}

void MainWindow::applyStyles(bool darkMode)
{
    // load the external stylesheet from resources
    QFile styleFile;

    if (darkMode) {
        styleFile.setFileName(":/styles/styleDark.qss");  // path to dark theme stylesheet
    } else {
        styleFile.setFileName(":/styles/style.qss");       // path to light theme stylesheet
    }

    qDebug() << "Loading stylesheet from:" << styleFile.fileName();

    if (styleFile.open(QFile::ReadOnly)) {
        // read the stylesheet content
        QString style = QLatin1String(styleFile.readAll());
        // close the file
        styleFile.close();
        // apply the stylesheet to the application
        qApp->setStyleSheet(style);
        // print a debug message indicating successful loading
        qDebug() << (darkMode ? "Dark mode" : "Light mode") << "stylesheet loaded successfully.";
    } else {
        // print a debug message if the stylesheet failed to load
        qDebug() << "Failed to load style.qss";
    }
}

// destructor for MainWindow
MainWindow::~MainWindow()
{
}

void MainWindow::toggleTheme()
{
    if (isUpdatingTheme) return; // Prevent recursive updates

    toggleDarkMode = !toggleDarkMode;
    isUpdatingTheme = true;

    if (toggleDarkMode) {
        clearInlineStyles(this); // clear inline styles for dark mode
        applyStyles(true);       // apply dark mode stylesheet
    } else {
        restoreInlineStyles(this); // restore original styles for light mode
        applyStyles(false);        // apply light mode stylesheet
    }

}

void MainWindow::clearInlineStyles(QWidget* widget) {
    // save the current style if it's not already stored
    if (!originalStyles.contains(widget)) {
        originalStyles[widget] = widget->styleSheet();
    }

    widget->setStyleSheet(""); // clear inline style for this widget

    // recursively clear child widgets
    for (auto child : widget->children()) {
        if (QWidget* childWidget = qobject_cast<QWidget*>(child)) {
            clearInlineStyles(childWidget);
        }
    }
}

void MainWindow::restoreInlineStyles(QWidget* widget) {
    if (originalStyles.contains(widget)) {
        widget->setStyleSheet(originalStyles[widget]); // restore original style from relevant ui file
    }

    // recursively restore child widgets
    for (auto child : widget->children()) {
        if (QWidget* childWidget = qobject_cast<QWidget*>(child)) {
            restoreInlineStyles(childWidget);
        }
    }
}


void MainWindow::navigateToDashboard()
{
    int totalReceipts = DatabaseManager::instance().getTotalReceipts(currentUserId);
    double totalSpending = DatabaseManager::instance().getTotalSpending(currentUserId);
    QList<QPair<QString, double>> spendingData = DatabaseManager::instance().getCategoryExpenses(currentUserId);
    QString topCategory = DatabaseManager::instance().getTopSpendingCategory(currentUserId);
    double avgMonthlySpending = DatabaseManager::instance().getAverageMonthlySpending(currentUserId);

    // Update the dashboard
    dashboardPage->updateDashboard(totalReceipts, totalSpending, avgMonthlySpending, topCategory, spendingData);

    // Set the current widget to dashboard
    stackedWidget->setCurrentWidget(dashboardPage);

}

// function to navigate back to the login page
void MainWindow::navigateToLogin()
{
    // set the current widget to the login page
    stackedWidget->setCurrentWidget(loginPage);
}


// function to handle receipt upload
void MainWindow::handleUploadReceipt()
{
    // get current user id
    int userId = getCurrentUserId();
    if (userId == -1) {
        QMessageBox::warning(this, "Error", "User not found. Please log in again.");
        return;
    }

    // open a file dialog to select PDF file
    QString fileName = QFileDialog::getOpenFileName(this, "Select Receipt PDF", "",
                                                    "Images (*.pdf *.jpg *.jpeg *.png);;All Files (*)");
    if (!fileName.isEmpty()) {
        // Absolute path to the project's Python venv interpreter. Using an
        // absolute path (not bare "python3") is required: a .app launched from
        // Finder has a minimal PATH and would otherwise hit the /usr/bin/python3
        // Xcode stub, which pops the "install command line developer tools" dialog.
        // Overridable via environment variable so this isn't locked to one
        // developer's machine; falls back to the original hardcoded path.
        QString pythonExecutable = qEnvironmentVariable("RECEIPTVAULT_PYTHON",
            "/Users/isaiah/Desktop/Career/Projects/receiptvault/backend/ml/venv/bin/python");
        QString scriptPath = qEnvironmentVariable("RECEIPTVAULT_ML_SCRIPT",
            "/Users/isaiah/Desktop/Career/Projects/receiptvault/backend/ml/experiments/LayoutLmV3_Inference_Script.py");

        // prepare the process arguments which includes the path the pdf
        QStringList arguments;
        arguments << scriptPath << fileName;

        // initialize the QProcess
        QProcess pythonProcess;
        pythonProcess.setProgram(pythonExecutable);
        pythonProcess.setArguments(arguments);

        // The first receipt scan cold-imports PyTorch and loads a ~500 MB model,
        // which can take well over the default 30s (especially while Spotlight is
        // still indexing the venv). Show a busy cursor and wait up to 5 minutes.
        QApplication::setOverrideCursor(Qt::WaitCursor);

        // start the Python script
        pythonProcess.start();
        if (!pythonProcess.waitForStarted(30000)) {
            QApplication::restoreOverrideCursor();
            QMessageBox::critical(this, "Error", "Failed to start Python process.");
            qDebug() << "Python process failed to start.";
            return;
        }

        // wait for the Python script to finish (up to 5 minutes)
        bool finished = pythonProcess.waitForFinished(300000);
        QApplication::restoreOverrideCursor();
        if (!finished) {
            QMessageBox::critical(this, "Error",
                                  "Receipt scanning timed out. Please try again "
                                  "(the first scan loads the model and is slowest).");
            qDebug() << "Python process did not finish within timeout.";
            return;
        }

        // read the standard output and error
        QString output = pythonProcess.readAllStandardOutput();
        QString errorOutput = pythonProcess.readAllStandardError();

        qDebug() << output << "Output from Python Script";
        if (!errorOutput.isEmpty())
            qDebug() << "Python stderr (non-fatal):" << errorOutput;

        // Only treat a non-zero exit code as failure. ML libraries emit harmless
        // warnings on stderr, so stderr being non-empty does NOT mean an error;
        // genuine failures are reported by the script as a non-zero exit.
        if (pythonProcess.exitCode() != 0) {
            QMessageBox::critical(this, "Python Error",
                                  errorOutput.isEmpty() ? output : errorOutput);
            qDebug() << "Python exited with code" << pythonProcess.exitCode();
            return;
        }


        // parse the JSON output
        QJsonDocument jsonDoc = QJsonDocument::fromJson(output.toUtf8());
        if (!jsonDoc.isObject()) {
            QMessageBox::critical(this, "Error", "Invalid JSON response from Python script.");
            qDebug() << "Invalid JSON:" << output;
            return;
        }

        qDebug() << "Parsed output to jsonDoc";

        QJsonObject jsonObj = jsonDoc.object();

        qDebug() << "Parsed output to jsonObject";

        //Parse store from json
        QString store = jsonObj["company"].toString();

        //parse total as a double from json
        QString totalString = jsonObj["total"].toString();

        qDebug() << "totalString";
        totalString.remove('$');
        double totalAmount = totalString.toDouble();
        qDebug() << "totalString good";

        // Parse date from JSON
        QString date = jsonObj["date"].toString();
        qDebug() << "Original date:" << date;

        // Replace any '/' with '-'
        date.replace("/", "-");

        qDebug() << "Date with '-': " << date;

        // Validate and format date
        QStringList dateParts = date.split('-');
        if (dateParts.size() == 3) {

            QString yearPart = dateParts[2];


            // Convert two digit year to standard full format
            if (yearPart.size() == 2) {
                int year = yearPart.toInt();
                if (year <= 50) {
                    yearPart = "20" + QString::number(year);
                } else {
                    yearPart = "19" + QString::number(year);
                }
                dateParts[2] = yearPart;
                qDebug() << "Converted two-digit year to four digits:" << yearPart;
            }

            if (dateParts[0].size() == 2 && dateParts[1].size() == 2 && dateParts[2].size() == 4) {
                // MM-DD-YYYY or DD-MM-YYYY format
                if (dateParts[0].toInt() <= 12) {
                    // Assume it's MM-DD-YYYY and flip to YYYY-MM-DD
                    date = dateParts[2] + '-' + dateParts[0] + '-' + dateParts[1];
                } else {
                    // Assume it's DD-MM-YYYY and flip to YYYY-MM-DD
                    date = dateParts[2] + '-' + dateParts[1] + '-' + dateParts[0];
                }
                qDebug() << "Formatted date:" << date;
            } else if (dateParts[0].size() == 4 && dateParts[1].size() == 2 && dateParts[2].size() == 2) {
                // Already in YYYY-MM-DD format
                qDebug() << "Date is already in YYYY-MM-DD format.";
            } else {
                // Invalid date parts size
                date = "";
                qDebug() << "Invalid date format. Resetting to empty string.";
            }
        } else {
            // If dateParts size is not 3 after splitting
            date = "";
            qDebug() << "Invalid date format with incorrect separators. Resetting to empty string.";
        }


        qDebug() << jsonObj;
        qDebug() << store;
        qDebug() << totalAmount;
        qDebug() << date;

        // assign a default category or allow the user to select
        QComboBox *categoryComboBox = new QComboBox(this);
        receiptsPage->populateCategoryComboBox(categoryComboBox);
        bool ok;
        int selectedCategoryId = categoryComboBox->currentData().toInt(&ok);
        if (!ok) {
            QMessageBox::warning(this, "Input Error", "Please select a valid category.");
            delete categoryComboBox;
            return;
        }

        // default description, could enhance
        QString description = "Receipt uploaded";

        // add the expense to the database
        bool success = DatabaseManager::instance().addExpense(userId, selectedCategoryId, store, date, totalAmount, description);

        if (success) {
            // retrieve the last inserted expense_id
            QSqlQuery query(DatabaseManager::instance().getDatabase());
            query.exec("SELECT last_insert_rowid()"); // For SQLite. Adjust if using another DB.
            int expenseId = -1;
            if (query.next()) {
                expenseId = query.value(0).toInt();
            }

            if(expenseId != -1){
                QString totalStr = QString::number(totalAmount, 'f', 2);
                receiptsPage->addReceipt(store, totalStr, date, selectedCategoryId, expenseId);

                QMessageBox::information(this, "Success", "Receipt uploaded successfully!");
            } else {
                QMessageBox::critical(this, "Error", "Failed to retrieve expense ID.");
                qDebug() << "Failed to retrieve expense ID.";
            }
        } else {
            QMessageBox::critical(this, "Database Error", "Failed to upload receipt.");
            qDebug() << "Failed to upload receipt to database.";
        }

        delete categoryComboBox; // Clean up
    } else {
        // user canceled the dialog
        qDebug() << "No file selected.";
    }
}

// function to get the current user's ID
int MainWindow::getCurrentUserId()
{
    return currentUserId;
}

void MainWindow::handleNavigateToAnalytics()
{
    int userId = getCurrentUserId();
    if (userId == -1) {
        QMessageBox::warning(this, "Error", "User not found. Please log in again.");
        return;
    }

    // fetch category expenses
    QList<QPair<QString, double>> categoryExpenses = DatabaseManager::instance().getCategoryExpenses(userId);

    // fetch monthly spending
    QList<QPair<QString, double>> monthlySpending = DatabaseManager::instance().getMonthlySpending(userId);

    // fetch top stores by spending
    QList<QPair<QString, double>> topStores = DatabaseManager::instance().getTopStores(userId, 5); // top 5 stores

    // update the AnalyticsPage with the fetched data
    analyticsPage->updateChartData(categoryExpenses, monthlySpending, topStores);

    // nav to the analytics
    stackedWidget->setCurrentWidget(analyticsPage);
}

void MainWindow::handleLogin(const QString &username, const QString &password)
{
    // Check if either the username or password fields are empty
    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please enter both username and password.");
        return;
    }

    // Fetch stored hashed password and salt
    QString storedHashedPassword, storedSalt;
    bool userExists = DatabaseManager::instance().getUserCredentials(username, storedHashedPassword, storedSalt);

    if (!userExists) {
        QMessageBox::warning(this, "Login Failed", "Incorrect username or password.");
        return;
    }

    // Hash the entered password with the stored salt
    QByteArray saltedPassword = password.toUtf8() + storedSalt.toUtf8();
    QByteArray hashedPassword = QCryptographicHash::hash(saltedPassword, QCryptographicHash::Sha256).toHex();

    // Verify user
    bool verified = DatabaseManager::instance().verifyUser(username, hashedPassword);

    if (verified) {
        currentUsername = username;

        // Retrieve and store user ID
        QSqlQuery query(DatabaseManager::instance().getDatabase());
        query.prepare("SELECT user_id FROM users WHERE username = :username");
        query.bindValue(":username", currentUsername);
        if (query.exec() && query.next()) {
            currentUserId = query.value("user_id").toInt();
        } else {
            qDebug() << "Failed to retrieve user ID:" << query.lastError().text();
            QMessageBox::critical(this, "Error", "Failed to retrieve user information.");
            return;
        }

        // Fetch dashboard data
        int totalReceipts = DatabaseManager::instance().getTotalReceipts(currentUserId);
        double totalSpending = DatabaseManager::instance().getTotalSpending(currentUserId);
        QString topCategory = DatabaseManager::instance().getTopSpendingCategory(currentUserId);
        QList<QPair<QString, double>> spendingData = DatabaseManager::instance().getCategoryExpenses(currentUserId);
        double avgMonthlySpending = DatabaseManager::instance().getAverageMonthlySpending(currentUserId);

        // Update the dashboard with fetched data
        dashboardPage->updateDashboard(totalReceipts, totalSpending, avgMonthlySpending, topCategory, spendingData);

        // Switch to the dashboard page
        stackedWidget->setCurrentWidget(dashboardPage);
    } else {
        QMessageBox::warning(this, "Login Failed", "Incorrect username or password.");
    }
}

void MainWindow::handleCreateAccount(const QString &username, const QString &password)
{

    // generate a unique salt (salting enhances the security of hashed passwords by adding random data)
    QString salt = QString::number(QDateTime::currentMSecsSinceEpoch());

    // hash the password with the salt using SHA-256
    QByteArray saltedPassword = password.toUtf8() + salt.toUtf8();
    QByteArray hashedPassword = QCryptographicHash::hash(saltedPassword, QCryptographicHash::Sha256).toHex();

    // insert into the database
    bool success = DatabaseManager::instance().createUser(username, hashedPassword, salt);

    if (success) {
        QMessageBox::information(this, "Success", "Account created successfully!");
        stackedWidget->setCurrentWidget(loginPage);
    } else {
        QMessageBox::critical(this, "Registration Error", "Failed to create account. Username might already exist.");
    }
}

void MainWindow::handleEditReceipt(int expenseId)
{
    // Fetch receipt details from the database
    QSqlQuery query(DatabaseManager::instance().getDatabase());
    query.prepare("SELECT store, expense_amount, expense_date, category_id FROM expenses WHERE expense_id = :expense_id AND user_id = :user_id");
    query.bindValue(":expense_id", expenseId);
    query.bindValue(":user_id", currentUserId);

    if (!query.exec() || !query.next()) {
        QMessageBox::critical(this, "Error", "Failed to retrieve receipt details.");
        return;
    }

    QString store = query.value("store").toString();
    double amount = query.value("expense_amount").toDouble();
    QString date = query.value("expense_date").toString();
    int categoryId = query.value("category_id").toInt();

    // Prompt user to edit details using QInputDialog
    bool ok;
    QString newStore = QInputDialog::getText(this, "Edit Store", "Store:", QLineEdit::Normal, store, &ok);
    if (!ok || newStore.isEmpty()) return;

    QString newAmountStr = QInputDialog::getText(this, "Edit Total", "Total Amount:", QLineEdit::Normal, QString::number(amount, 'f', 2), &ok);
    if (!ok) return;
    double newAmount = newAmountStr.toDouble(&ok);
    if (!ok || newAmount <= 0) {
        QMessageBox::warning(this, "Input Error", "Please enter a valid amount.");
        return;
    }

    QString newDate = QInputDialog::getText(this, "Edit Date", "Date (YYYY-MM-DD):", QLineEdit::Normal, date, &ok);
    if (!ok || !QDate::fromString(newDate, "yyyy-MM-dd").isValid()) {
        QMessageBox::warning(this, "Input Error", "Please enter a valid date.");
        return;
    }

    // Update the database
    QSqlQuery updateQuery(DatabaseManager::instance().getDatabase());
    updateQuery.prepare(R"(
        UPDATE expenses
        SET store = :store, expense_amount = :amount, expense_date = :date, category_id = :category_id
        WHERE expense_id = :expense_id AND user_id = :user_id
    )");
    updateQuery.bindValue(":store", newStore);
    updateQuery.bindValue(":amount", newAmount);
    updateQuery.bindValue(":date", newDate);
    updateQuery.bindValue(":category_id", categoryId); // Update as needed
    updateQuery.bindValue(":expense_id", expenseId);
    updateQuery.bindValue(":user_id", currentUserId);

    if (updateQuery.exec()) {
        QMessageBox::information(this, "Success", "Receipt updated successfully!");
        receiptsPage->loadReceipts(currentUserId);
    } else {
        QMessageBox::critical(this, "Database Error", "Failed to update receipt.");
        qDebug() << "Update error:" << updateQuery.lastError().text();
    }
}
