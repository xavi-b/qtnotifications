#include "qplatformnotificationengine_linux.h"
#include <QtDBus/QtDBus>

QT_BEGIN_NAMESPACE

QPlatformNotificationEngineLinux::QPlatformNotificationEngineLinux(QObject *parent)
: QPlatformNotificationEngine(parent)
{
    QDBusConnection::sessionBus().connect(
        QStringLiteral("org.freedesktop.Notifications"),
        QStringLiteral("/org/freedesktop/Notifications"),
        QStringLiteral("org.freedesktop.Notifications"),
        QStringLiteral("ActionInvoked"),
        this,
        SLOT(onActionInvoked(uint, QString))
    );
    QDBusConnection::sessionBus().connect(
        QStringLiteral("org.freedesktop.Notifications"),
        QStringLiteral("/org/freedesktop/Notifications"),
        QStringLiteral("org.freedesktop.Notifications"),
        QStringLiteral("NotificationClosed"),
        this,
        SLOT(onNotificationClosed(uint, uint))
    );
}

bool QPlatformNotificationEngineLinux::isSupported() const
{
    return QDBusConnection::sessionBus().isConnected();
}

bool QPlatformNotificationEngineLinux::sendNotification(const QString &summary, const QString &body, const QString &icon, const QMap<QString, QString> &actions, QNotifications::NotificationType type)
{
    QDBusMessage msg = QDBusMessage::createMethodCall(
        QStringLiteral("org.freedesktop.Notifications"),
        QStringLiteral("/org/freedesktop/Notifications"),
        QStringLiteral("org.freedesktop.Notifications"),
        QStringLiteral("Notify"));

    QList<QVariant> args;
    QStringList actionList;

    // Add default action to make notification clickable
    actionList << QStringLiteral("default") << QStringLiteral("");

    // Add user-defined actions
    for (auto it = actions.constBegin(); it != actions.constEnd(); ++it)
        actionList << it.key() << it.value();

    int urgency = 0;

    if (type == QNotifications::Warning || type == QNotifications::Error)
        urgency = 2;
    else if (type == QNotifications::Success)
        urgency = 1;

    QVariantMap map;
    map.insert(QStringLiteral("urgency"), urgency);

    args << QStringLiteral("qtnotifications")
         << uint(0)
         << icon
         << summary
         << body
         << QVariant::fromValue(actionList)
         << map
         << int(5000);

    msg.setArguments(args);
    QDBusMessage reply = QDBusConnection::sessionBus().call(msg);
    if (reply.type() == QDBusMessage::ReplyMessage && !reply.arguments().isEmpty())
        return true;
    return false;
}

void QPlatformNotificationEngineLinux::onActionInvoked(uint id, const QString &actionKey)
{
    // Check if this is a notification click (default action) vs a specific action button
    if (actionKey == QStringLiteral("default")) {
        emit notificationClicked(id);
    } else {
        emit actionInvoked(id, actionKey);
    }
}

void QPlatformNotificationEngineLinux::onNotificationClosed(uint id, uint reason)
{
    QNotifications::ClosedReason closedReason;
    switch (reason) {
        case 1:
            closedReason = QNotifications::Expired;
            break;
        case 2:
            closedReason = QNotifications::Dismissed;
            break;
        case 3:
            closedReason = QNotifications::Closed;
            break;
        default:
            closedReason = QNotifications::Undefined;
            break;
    }
    emit notificationClosed(id, closedReason);
}

QPlatformNotificationEngine *qt_create_notification_engine_linux()
{
    static QPlatformNotificationEngineLinux engine;
    return &engine;
}

QT_END_NAMESPACE
