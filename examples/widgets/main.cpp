#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QMessageBox>
#include <QtCore/QTimer>
#include <QtCore/QDateTime>
#include <QtWidgets/QFileDialog>
#include <QtCore/QDir>
#include <qnotifications.h>
#include <QtCore/QDebug>

class NotificationsWidget : public QMainWindow
{
public:
    NotificationsWidget(QWidget *parent = nullptr)
        : QMainWindow(parent)
        , notifications(new QNotifications(this))
    {
        setWindowTitle("Qt Notifications Widgets Example: " + qApp->organizationDomain() + "." + qApp->applicationName());
        setMinimumSize(600, 500);

        // Check if notifications are supported
        if (!notifications->isSupported()) {
            QMessageBox::warning(this, "Not Supported",
                "Notifications are not supported on this platform!");
        }

        setupUI();
        setupConnections();
    }

private:
    void sendSimpleNotification()
    {
        bool success = notifications->sendNotification(
            titleEdit->text().isEmpty() ? "Qt Notifications Test" : titleEdit->text(),
            messageEdit->toPlainText().isEmpty() ? "This is a test notification from the Qt Notifications module!" : messageEdit->toPlainText(),
            getNotificationType()
        );

        updateStatus(success ? "Simple notification sent successfully!" : "Failed to send simple notification");
    }

    void sendNotificationWithIcon()
    {
        bool success = notifications->sendNotification(
            titleEdit->text().isEmpty() ? "Qt Notifications Test 2" : titleEdit->text(),
            messageEdit->toPlainText().isEmpty() ? "This notification includes an icon path (if supported)" : messageEdit->toPlainText(),
            iconPathEdit->text().isEmpty() ? "/path/to/icon.png" : iconPathEdit->text(),
            getNotificationType()
        );

        updateStatus(success ? "Notification with icon sent successfully!" : "Failed to send notification with icon");
    }

    void sendNotificationWithActions()
    {
        QMap<QString, QString> actions;
        actions["open"] = "Open";
        actions["dismiss"] = "Dismiss";
        actions["reply"] = "Reply";

        bool success = notifications->sendNotification(
            titleEdit->text().isEmpty() ? "Qt Notifications with Actions" : titleEdit->text(),
            messageEdit->toPlainText().isEmpty() ? "Choose an action below:" : messageEdit->toPlainText(),
            actions,
            getNotificationType()
        );

        updateStatus(success ? "Notification with actions sent successfully!" : "Failed to send notification with actions");
    }

    void onNotificationClosed()
    {
        updateStatus("Notification was closed!");
    }

    void onActionInvoked(uint notificationId, const QString &actionKey)
    {
        QString actionText;
        if (actionKey == "open") {
            actionText = "Opening application...";
        } else if (actionKey == "dismiss") {
            actionText = "Dismissing notification...";
        } else if (actionKey == "reply") {
            actionText = "Opening reply dialog...";
        } else {
            actionText = QString("Unknown action: %1").arg(actionKey);
        }

        updateStatus(QString("Action invoked for notification %1: %2").arg(notificationId).arg(actionText));
    }

    void onNotificationClicked(uint notificationId)
    {
        updateStatus(QString("Notification %1 was clicked!").arg(notificationId));
    }

    void browseIconFile()
    {
        QString fileName = QFileDialog::getOpenFileName(this,
            tr("Select Icon File"),
            QDir::homePath(),
            tr("Image Files (*.png *.jpg *.jpeg *.bmp *.ico *.svg);;All Files (*)"));

        if (!fileName.isEmpty()) {
            iconPathEdit->setText(fileName);
            updateStatus(QString("Icon file selected: %1").arg(fileName));
        }
    }

private:
    void setupUI()
    {
        QWidget *centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);

        QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

        // Title and message input group
        QGroupBox *inputGroup = new QGroupBox("Notification Content", this);
        QVBoxLayout *inputLayout = new QVBoxLayout(inputGroup);

        // Title input
        QHBoxLayout *titleLayout = new QHBoxLayout();
        titleLayout->addWidget(new QLabel("Title:", this));
        titleEdit = new QLineEdit(this);
        titleEdit->setPlaceholderText("Enter notification title...");
        titleLayout->addWidget(titleEdit);
        inputLayout->addLayout(titleLayout);

        // Message input
        QHBoxLayout *messageLayout = new QHBoxLayout();
        messageLayout->addWidget(new QLabel("Message:", this));
        messageEdit = new QTextEdit(this);
        messageEdit->setMaximumHeight(80);
        messageEdit->setPlaceholderText("Enter notification message...");
        messageLayout->addWidget(messageEdit);
        inputLayout->addLayout(messageLayout);

        // Icon path input
        QHBoxLayout *iconLayout = new QHBoxLayout();
        iconLayout->addWidget(new QLabel("Icon Path:", this));
        iconPathEdit = new QLineEdit(this);
        iconPathEdit->setPlaceholderText("/path/to/icon.png");
        iconLayout->addWidget(iconPathEdit);

        QPushButton *browseBtn = new QPushButton("Browse...", this);
        connect(browseBtn, &QPushButton::clicked, this, &NotificationsWidget::browseIconFile);
        iconLayout->addWidget(browseBtn);

        inputLayout->addLayout(iconLayout);

        // Notification type selector
        QHBoxLayout *typeLayout = new QHBoxLayout();
        typeLayout->addWidget(new QLabel("Type:", this));
        typeCombo = new QComboBox(this);
        typeCombo->addItem("Information", QNotifications::Information);
        typeCombo->addItem("Success", QNotifications::Success);
        typeCombo->addItem("Warning", QNotifications::Warning);
        typeCombo->addItem("Error", QNotifications::Error);
        typeLayout->addWidget(typeCombo);
        inputLayout->addLayout(typeLayout);

        mainLayout->addWidget(inputGroup);

        // Buttons group
        QGroupBox *buttonsGroup = new QGroupBox("Send Notifications", this);
        QVBoxLayout *buttonsLayout = new QVBoxLayout(buttonsGroup);

        QPushButton *simpleBtn = new QPushButton("Send Simple Notification", this);
        QPushButton *iconBtn = new QPushButton("Send Notification with Icon", this);
        QPushButton *actionsBtn = new QPushButton("Send Notification with Actions", this);

        buttonsLayout->addWidget(simpleBtn);
        buttonsLayout->addWidget(iconBtn);
        buttonsLayout->addWidget(actionsBtn);

        mainLayout->addWidget(buttonsGroup);

        // Status display
        QGroupBox *statusGroup = new QGroupBox("Status Log", this);
        QVBoxLayout *statusLayout = new QVBoxLayout(statusGroup);
        statusText = new QTextEdit(this);
        statusText->setReadOnly(true);
        statusText->setMaximumHeight(150);
        statusLayout->addWidget(statusText);
        mainLayout->addWidget(statusGroup);

        // Connect button signals
        connect(simpleBtn, &QPushButton::clicked, this, &NotificationsWidget::sendSimpleNotification);
        connect(iconBtn, &QPushButton::clicked, this, &NotificationsWidget::sendNotificationWithIcon);
        connect(actionsBtn, &QPushButton::clicked, this, &NotificationsWidget::sendNotificationWithActions);
    }

    void setupConnections()
    {
        connect(notifications, &QNotifications::notificationClosed,
                this, &NotificationsWidget::onNotificationClosed);
        connect(notifications, &QNotifications::actionInvoked,
                this, &NotificationsWidget::onActionInvoked);
        connect(notifications, &QNotifications::notificationClicked,
                this, &NotificationsWidget::onNotificationClicked);
    }

    QNotifications::NotificationType getNotificationType()
    {
        return static_cast<QNotifications::NotificationType>(typeCombo->currentData().toInt());
    }

    void updateStatus(const QString &message)
    {
        QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
        statusText->append(QString("[%1] %2").arg(timestamp, message));
        qInfo() << message;
    }

private:
    QNotifications *notifications;
    QLineEdit *titleEdit;
    QTextEdit *messageEdit;
    QLineEdit *iconPathEdit;
    QComboBox *typeCombo;
    QTextEdit *statusText;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    NotificationsWidget widget;
    widget.show();

    return app.exec();
}
