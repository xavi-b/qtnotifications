#include "qdeclarativenotifications_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype Notifications
    \inqmlmodule QtNotifications
    \brief Provides a QML API for sending system notifications.

    The Notifications type allows you to send notifications to the user's desktop
    notification system from QML.

    \section1 Basic Usage

    \qml
    import QtNotifications

    Notifications {
        id: notifications

        Component.onCompleted: {
            if (notifications.isSupported()) {
                notifications.sendNotification(
                    "Hello",
                    "This is a test notification"
                );
            }
        }

        onNotificationClicked: function(notificationId) {
            console.log("Notification clicked:", notificationId);
        }
    }
    \endqml

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
            notifications.sendNotification("Title", "Message", {}, actions);
        }

        onActionInvoked: function(notificationId, actionKey) {
            console.log("Action invoked:", actionKey);
        }
    }
    \endqml

    \sa QNotifications
*/

/*!
    \qmlproperty enumeration Notifications::ClosedReason

    This property holds the reason for the notification being closed. It can be one of the following.

    \qmlenumeratorsfrom QNotifications::ClosedReason
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
    \qmlsignal Notifications::notificationClosed(uint notificationId, QNotifications::ClosedReason reason)

    This signal is emitted when a notification is closed by the user or the system.

    \a notificationId is the ID of the notification that was closed.
    \a reason is the reason for the notification being closed.

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
    \qmlmethod uint Notifications::sendNotification(string title, string message, var parameters, var actions)

    Sends a notification with the given \a title, \a message, \a parameters, and \a actions.

    Returns the ID of the notification that was sent.
*/
uint QDeclarativeNotifications::sendNotification(const QString &title, const QString &message, const QVariantMap &parameters, const QVariantMap &actions)
{
    QMap<QString, QString> stringMap;
    for (auto it = actions.constBegin(); it != actions.constEnd(); ++it) {
        stringMap.insert(it.key(), it.value().toString());
    }
    return m_notifications.sendNotification(title, message, parameters, stringMap);
}

QT_END_NAMESPACE
