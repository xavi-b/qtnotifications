#include <QtCore/QCoreApplication>
#include <QtCore/QTimer>
#include <qnotifications.h>
#include <QtCore/QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QNotifications notifications;

    if (!notifications.isSupported()) {
        qWarning() << "Notifications are not supported on this platform";
        return 1;
    }

    qInfo() << "Sending test notification...";

    // Send a simple notification
    bool success = notifications.sendNotification(
        "Qt Notifications Test",
        "This is a test notification from the Qt Notifications module!",
        QNotifications::Information
    );

    if (success) {
        qInfo() << "Notification sent successfully!";
    } else {
        qWarning() << "Failed to send notification";
    }

    // Send another notification with an icon
    success = notifications.sendNotification(
        "Qt Notifications Test 2",
        "This notification includes an icon path (if supported)",
        "/path/to/icon.png",
        QNotifications::Success
    );

    if (success) {
        qInfo() << "Second notification sent successfully!";
    } else {
        qWarning() << "Failed to send second notification";
    }

    // Send a notification with actions
    QMap<QString, QString> actions;
    actions["open"] = "Open";
    actions["dismiss"] = "Dismiss";
    actions["reply"] = "Reply";
    success = notifications.sendNotification(
        "Qt Notifications with Actions",
        "Choose an action below:",
        actions,
        QNotifications::Information
    );
    if (success) {
        qInfo() << "Notification with actions sent successfully!";
    } else {
        qWarning() << "Failed to send notification with actions";
    }

    // Connect to notification signals
    QObject::connect(&notifications, &QNotifications::notificationClosed,
                     []() { qInfo() << "Notification was closed!"; });

    // Connect to actionInvoked signal
    QObject::connect(&notifications, &QNotifications::actionInvoked,
                     [](uint notificationId, const QString &actionKey) {
                         qInfo() << "Action invoked for notification" << notificationId << "with key:" << actionKey;
                         if (actionKey == "open") {
                             qInfo() << "Opening application...";
                         } else if (actionKey == "dismiss") {
                             qInfo() << "Dismissing notification...";
                         } else if (actionKey == "reply") {
                             qInfo() << "Opening reply dialog...";
                         }
                     });

    return app.exec();
}
