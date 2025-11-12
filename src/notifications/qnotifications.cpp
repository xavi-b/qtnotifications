#include "qnotifications.h"
#include "qplatformnotificationengine.h"

QT_BEGIN_NAMESPACE

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

bool QNotifications::isSupported() const
{
    return m_engine && m_engine->isSupported();
}

bool QNotifications::sendNotification(const QString &title,
                                     const QString &message,
                                     NotificationType type)
{
    return sendNotification(title, message, QString(), {}, type);
}

bool QNotifications::sendNotification(const QString &title,
                                     const QString &message,
                                     const QString &iconPath,
                                     NotificationType type)
{
    return sendNotification(title, message, iconPath, {}, type);
}

bool QNotifications::sendNotification(const QString &title,
                                     const QString &message,
                                     const QMap<QString, QString> &actions,
                                     NotificationType type)
{
    return sendNotification(title, message, QString(), actions, type);
}

bool QNotifications::sendNotification(const QString &title,
                                     const QString &message,
                                     const QString &iconPath,
                                     const QMap<QString, QString> &actions,
                                     NotificationType type)
{
    if (!m_engine)
        return false;
    return m_engine->sendNotification(title, message, iconPath, actions, static_cast<int>(type));
}

QT_END_NAMESPACE

#include "moc_qnotifications.cpp"
