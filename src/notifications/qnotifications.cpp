#include "qnotifications.h"
#include "qplatformnotificationengine.h"

QT_BEGIN_NAMESPACE

/*!
    \class QNotifications
    \inmodule QtNotifications
    \brief The QNotifications class provides a cross-platform API for sending system notifications.

    QNotifications allows you to send notifications to the user's desktop notification system.
    It supports different notification types, custom icons, and action buttons.

    \section1 Basic Usage

    \code
    QNotifications notifications;
    if (notifications.isSupported()) {
        notifications.sendNotification(
            "Hello",
            "This is a test notification",
            QNotifications::Information
        );
    }
    \endcode

    \section1 Actions

    Notifications can include action buttons that the user can interact with.
    Actions are provided as a QMap where the key is the action identifier and
    the value is the display text.

    \code
    QMap<QString, QString> actions;
    actions["open"] = "Open";
    actions["dismiss"] = "Dismiss";
    notifications.sendNotification("Title", "Message", actions);
    \endcode

    When an action is invoked, the \l actionInvoked() signal is emitted.

    \sa NotificationType
*/

/*!
    \enum QNotifications::NotificationType

    This enum describes the type of notification.

    \value Information
        General informational notification.
    \value Warning
        Warning notification.
    \value Error
        Error notification.
    \value Success
        Success notification.
*/

/*!
    \enum QNotifications::ClosedReason

    This enum describes the type of notification.

    \value Expired
        The notification timed out and was closed automatically by the server.
    \value Dismissed
        The user dismissed the notification.
    \value Closed
        The notification was closed programmatically.
    \value Undefined
        The reason for the notification being closed is unknown.
*/

/*!
    \fn QNotifications::actionInvoked(uint notificationId, const QString &actionKey)

    This signal is emitted when a notification action is invoked by the user.

    \a notificationId is the ID of the notification that triggered the action.
    \a actionKey is the key of the action that was invoked, as specified in the
    actions map when sending the notification.

    \sa sendNotification()
*/

/*!
    \fn QNotifications::notificationClosed(uint notificationId, ClosedReason reason)

    This signal is emitted when a notification is closed by the user or the system.

    \a notificationId is the ID of the notification that was closed.
    \a reason is the reason for the notification being closed.

    \sa sendNotification()
*/

/*!
    \fn QNotifications::notificationClicked(uint notificationId)

    This signal is emitted when a notification is clicked by the user.

    \a notificationId is the ID of the notification that was clicked.

    \sa sendNotification()
*/

QNotifications::QNotifications(QObject *parent)
    : QObject(parent)
    , m_engine(qt_notification_engine())
{
    if (m_engine) {
        connect(m_engine, &QPlatformNotificationEngine::actionInvoked, this, &QNotifications::actionInvoked);
        connect(m_engine, &::QPlatformNotificationEngine::notificationClosed, this, &QNotifications::notificationClosed);
        connect(m_engine, &QPlatformNotificationEngine::notificationClicked, this, &QNotifications::notificationClicked);
    }
}

QNotifications::~QNotifications() = default;

/*!
    Returns \c true if notifications are supported on the current platform;
    otherwise returns \c false.

    \sa sendNotification()
*/
bool QNotifications::isSupported() const
{
    return m_engine && m_engine->isSupported();
}

/*!
    Sends a notification with the given \a title, \a message, and \a type.

    Returns \c true if the notification was sent successfully; otherwise returns \c false.

    \sa NotificationType
*/
bool QNotifications::sendNotification(const QString &title,
                                     const QString &message,
                                     NotificationType type)
{
    return sendNotification(title, message, QString(), {}, type);
}

/*!
    Sends a notification with the given \a title, \a message, \a iconPath, and \a type.

    The \a iconPath should be a path to an image file. The format and size requirements
    depend on the platform.

    Returns \c true if the notification was sent successfully; otherwise returns \c false.

    \sa NotificationType
*/
bool QNotifications::sendNotification(const QString &title,
                                     const QString &message,
                                     const QString &iconPath,
                                     NotificationType type)
{
    return sendNotification(title, message, iconPath, {}, type);
}

/*!
    Sends a notification with the given \a title, \a message, \a actions, and \a type.

    The \a actions parameter is a map where keys are action identifiers and values
    are the display text for the action buttons.

    Returns \c true if the notification was sent successfully; otherwise returns \c false.

    When an action is invoked, the \l actionInvoked() signal is emitted with the
    notification ID and the action key.

    \sa actionInvoked(), NotificationType
*/
bool QNotifications::sendNotification(const QString &title,
                                     const QString &message,
                                     const QMap<QString, QString> &actions,
                                     NotificationType type)
{
    return sendNotification(title, message, QString(), actions, type);
}

/*!
    Sends a notification with the given \a title, \a message, \a iconPath, \a actions, and \a type.

    This is the most complete form of sendNotification(), including all optional features.

    Returns \c true if the notification was sent successfully; otherwise returns \c false.

    \sa actionInvoked(), NotificationType
*/
bool QNotifications::sendNotification(const QString &title,
                                     const QString &message,
                                     const QString &iconPath,
                                     const QMap<QString, QString> &actions,
                                     NotificationType type)
{
    if (!m_engine)
        return false;
    return m_engine->sendNotification(title, message, iconPath, actions, type);
}

QT_END_NAMESPACE

#include "moc_qnotifications.cpp"
