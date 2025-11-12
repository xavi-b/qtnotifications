#include "qdeclarativenotifications_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype Notifications
    \inqmlmodule QtNotifications
    \brief Provides a QML API for sending system notifications.

    The Notifications type allows you to send notifications to the user's desktop
    notification system from QML. It supports different notification types, custom
    icons, and action buttons.

    \section1 Basic Usage

    \qml
    import QtNotifications

    Notifications {
        id: notifications

        Component.onCompleted: {
            if (notifications.isSupported()) {
                notifications.sendNotification(
                    "Hello",
                    "This is a test notification",
                    0  // Information type
                );
            }
        }

        onNotificationClicked: function(notificationId) {
            console.log("Notification clicked:", notificationId);
        }
    }
    \endqml

    \section1 Notification Types

    The notification type is specified as a QNotifications::NotificationType:
    \list
    \li \c QNotifications::Information - Information (default)
    \li \c QNotifications::Warning - Warning
    \li \c QNotifications::Error - Error
    \li \c QNotifications::Success - Success
    \endlist

    \section1 Actions

    Notifications can include action buttons. Actions are provided as a JavaScript
    object where keys are action identifiers and values are the display text.

    \qml
    Notifications {
        id: notifications

        function sendWithActions() {
            var actions = {
                "open": "Open",
                "dismiss": "Dismiss"
            };
            notifications.sendNotification("Title", "Message", actions);
        }

        onActionInvoked: function(notificationId, actionKey) {
            console.log("Action invoked:", actionKey);
        }
    }
    \endqml

    \sa QNotifications
*/

/*!
    \qmlproperty enumeration Notifications::NotificationType

    This property holds the type of notification. It can be one of the following.

    \qmlenumeratorsfrom QNotifications::NotificationType
*/

/*!
    \qmlsignal Notifications::actionInvoked(uint notificationId, string actionKey)

    This signal is emitted when a notification action is invoked by the user.

    \a notificationId is the ID of the notification that triggered the action.
    \a actionKey is the key of the action that was invoked, as specified in the
    actions object when sending the notification.

    \sa sendNotification()
*/

/*!
    \qmlsignal Notifications::notificationClosed(uint notificationId)

    This signal is emitted when a notification is closed by the user or the system.

    \a notificationId is the ID of the notification that was closed.

    \sa sendNotification()
*/

/*!
    \qmlsignal Notifications::notificationClicked(uint notificationId)

    This signal is emitted when a notification is clicked by the user.

    \a notificationId is the ID of the notification that was clicked.

    \sa sendNotification()
*/

QDeclarativeNotifications::QDeclarativeNotifications(QObject *parent)
    : QObject(parent)
{
    connect(&m_notifications, &QNotifications::actionInvoked, this, &QDeclarativeNotifications::actionInvoked);
    connect(&m_notifications, &QNotifications::notificationClosed, this, &QDeclarativeNotifications::notificationClosed);
    connect(&m_notifications, &QNotifications::notificationClicked, this, &QDeclarativeNotifications::notificationClicked);
}

/*!
    \qmlmethod bool Notifications::isSupported()

    Returns \c true if notifications are supported on the current platform;
    otherwise returns \c false.

    \sa sendNotification()
*/
bool QDeclarativeNotifications::isSupported() const
{
    return m_notifications.isSupported();
}

/*!
    \qmlmethod bool Notifications::sendNotification(string title, string message, NotificationType type)

    Sends a notification with the given \a title, \a message, and \a type.

    Returns \c true if the notification was sent successfully; otherwise returns \c false.

    \sa QNotifications::NotificationType
*/
bool QDeclarativeNotifications::sendNotification(const QString &title, const QString &message, QNotifications::NotificationType type)
{
    return m_notifications.sendNotification(title, message, static_cast<QNotifications::NotificationType>(type));
}

/*!
    \qmlmethod bool Notifications::sendNotification(string title, string message, string iconPath, NotificationType type)

    Sends a notification with the given \a title, \a message, \a iconPath, and \a type.

    The \a iconPath should be a path to an image file. The format and size requirements
    depend on the platform.

    Returns \c true if the notification was sent successfully; otherwise returns \c false.

    \sa QNotifications::NotificationType
*/
bool QDeclarativeNotifications::sendNotification(const QString &title, const QString &message, const QString &iconPath, QNotifications::NotificationType type)
{
    return m_notifications.sendNotification(title, message, iconPath, static_cast<QNotifications::NotificationType>(type));
}

/*!
    \qmlmethod bool Notifications::sendNotification(string title, string message, var actions, NotificationType type)

    Sends a notification with the given \a title, \a message, \a actions, and \a type.

    The \a actions parameter should be a JavaScript object where keys are action
    identifiers and values are the display text for the action buttons.

    Returns \c true if the notification was sent successfully; otherwise returns \c false.

    When an action is invoked, the \l actionInvoked() signal is emitted.

    \sa actionInvoked(), QNotifications::NotificationType
*/
bool QDeclarativeNotifications::sendNotification(const QString &title, const QString &message, const QVariantMap &actions, QNotifications::NotificationType type)
{
    QMap<QString, QString> stringMap;
    for (auto it = actions.constBegin(); it != actions.constEnd(); ++it) {
        stringMap.insert(it.key(), it.value().toString());
    }
    return m_notifications.sendNotification(title, message, stringMap, static_cast<QNotifications::NotificationType>(type));
}

/*!
    \qmlmethod bool Notifications::sendNotification(string title, string message, string iconPath, var actions, NotificationType type)

    Sends a notification with the given \a title, \a message, \a iconPath, \a actions, and \a type.

    This is the most complete form of sendNotification(), including all optional features.

    Returns \c true if the notification was sent successfully; otherwise returns \c false.

    \sa actionInvoked(), QNotifications::NotificationType
*/
bool QDeclarativeNotifications::sendNotification(const QString &title, const QString &message, const QString &iconPath, const QMap<QString, QString> &actions, QNotifications::NotificationType type)
{
    return m_notifications.sendNotification(title, message, iconPath, actions, static_cast<QNotifications::NotificationType>(type));
}

QT_END_NAMESPACE
