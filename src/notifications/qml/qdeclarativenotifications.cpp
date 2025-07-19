#include "qdeclarativenotifications_p.h"

QT_BEGIN_NAMESPACE

QDeclarativeNotifications::QDeclarativeNotifications(QObject *parent)
    : QObject(parent)
{
    connect(&m_notifications, &QNotifications::actionInvoked, this, &QDeclarativeNotifications::actionInvoked);
    connect(&m_notifications, &QNotifications::notificationClosed, this, &QDeclarativeNotifications::notificationClosed);
}

bool QDeclarativeNotifications::isSupported() const
{
    return m_notifications.isSupported();
}

bool QDeclarativeNotifications::sendNotification(const QString &title, const QString &message, int type)
{
    return m_notifications.sendNotification(title, message, static_cast<QNotifications::NotificationType>(type));
}

bool QDeclarativeNotifications::sendNotification(const QString &title, const QString &message, const QString &iconPath, int type)
{
    return m_notifications.sendNotification(title, message, iconPath, static_cast<QNotifications::NotificationType>(type));
}

bool QDeclarativeNotifications::sendNotification(const QString &title, const QString &message, const QVariantMap &actions, int type)
{
    QMap<QString, QString> stringMap;
    for (auto it = actions.constBegin(); it != actions.constEnd(); ++it) {
        stringMap.insert(it.key(), it.value().toString());
    }
    return m_notifications.sendNotification(title, message, stringMap, static_cast<QNotifications::NotificationType>(type));
}

bool QDeclarativeNotifications::sendNotification(const QString &title, const QString &message, const QString &iconPath, const QMap<QString, QString> &actions, int type)
{
    return m_notifications.sendNotification(title, message, iconPath, actions, static_cast<QNotifications::NotificationType>(type));
}

QT_END_NAMESPACE
