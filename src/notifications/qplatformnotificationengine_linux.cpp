#include "qplatformnotificationengine_linux.h"
#include <QtDBus/QtDBus>

QT_BEGIN_NAMESPACE

QPlatformNotificationEngineLinux::QPlatformNotificationEngineLinux(QObject *parent)
: QPlatformNotificationEngine(parent)
{
    QDBusConnection::sessionBus().connect(
        "org.freedesktop.Notifications",
        "/org/freedesktop/Notifications",
        "org.freedesktop.Notifications",
        "ActionInvoked",
        this,
        SLOT(onActionInvoked(uint, QString))
    );
    QDBusConnection::sessionBus().connect(
        "org.freedesktop.Notifications",
        "/org/freedesktop/Notifications",
        "org.freedesktop.Notifications",
        "NotificationClosed",
        this,
        SLOT(onNotificationClosed(uint, uint))
    );
}

bool QPlatformNotificationEngineLinux::isSupported() const
{
    return QDBusConnection::sessionBus().isConnected();
}

bool QPlatformNotificationEngineLinux::sendNotification(const QString &summary, const QString &body, const QString &icon, const QMap<QString, QString> &actions, int type)
{
    Q_UNUSED(type);

    QDBusMessage msg = QDBusMessage::createMethodCall(
        "org.freedesktop.Notifications",
        "/org/freedesktop/Notifications",
        "org.freedesktop.Notifications",
        "Notify");

    QList<QVariant> args;
    QStringList actionList;
    for (auto it = actions.constBegin(); it != actions.constEnd(); ++it)
        actionList << it.key() << it.value();
    args << QString("qtnotifications")
         << uint(0)
         << icon
         << summary
         << body
         << QVariant::fromValue(actionList)
         << QVariantMap()
         << int(5000);

    msg.setArguments(args);
    QDBusMessage reply = QDBusConnection::sessionBus().call(msg);
    if (reply.type() == QDBusMessage::ReplyMessage && !reply.arguments().isEmpty())
        return true;
    return false;
}

void QPlatformNotificationEngineLinux::onActionInvoked(uint id, const QString &actionKey)
{
    emit actionInvoked(id, actionKey);
}

void QPlatformNotificationEngineLinux::onNotificationClosed(uint id, uint reason)
{
    emit notificationClosed(id, reason);
}

QPlatformNotificationEngine *qt_create_notification_engine_linux()
{
    static QPlatformNotificationEngineLinux engine;
    return &engine;
}

QT_END_NAMESPACE
