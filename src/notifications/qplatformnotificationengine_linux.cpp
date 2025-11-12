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

uint QPlatformNotificationEngineLinux::sendNotification(const QString &title, const QString &message, const QVariantMap &parameters, const QMap<QString, QString> &actions)
{
    int urgency = parameters.value(QStringLiteral("urgency")).toInt();
    QString icon = parameters.value(QStringLiteral("icon")).toString();
    int expireTimeout = parameters.value(QStringLiteral("expire-timeout"), -1).toInt();

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

    QVariantMap map;
    map.insert(QStringLiteral("urgency"), urgency);

    // Handle image-data structure (iiibiiay)
    QVariant imageDataParam = parameters.value(QStringLiteral("image-data"));
    if (imageDataParam.isValid()) {
        if (imageDataParam.typeId() == QMetaType::QVariantMap) {
            // Build DBus structure (iiibiiay) from QVariantMap
            QVariantMap imageDataMap = imageDataParam.toMap();
            QDBusArgument imageDataArg;
            imageDataArg.beginStructure();
            imageDataArg << imageDataMap.value(QStringLiteral("width")).toInt();
            imageDataArg << imageDataMap.value(QStringLiteral("height")).toInt();
            imageDataArg << imageDataMap.value(QStringLiteral("rowstride")).toInt();
            imageDataArg << imageDataMap.value(QStringLiteral("has_alpha")).toBool();
            imageDataArg << imageDataMap.value(QStringLiteral("bits_per_sample")).toInt();
            imageDataArg << imageDataMap.value(QStringLiteral("channels")).toInt();
            imageDataArg << imageDataMap.value(QStringLiteral("data")).toByteArray();
            imageDataArg.endStructure();
            map.insert(QStringLiteral("image-data"), QVariant::fromValue(imageDataArg));
        }
    }

    args << QStringLiteral("qtnotifications")
         << uint(0)
         << icon
         << title
         << message
         << QVariant::fromValue(actionList)
         << map
         << expireTimeout;

    msg.setArguments(args);
    QDBusMessage reply = QDBusConnection::sessionBus().call(msg);
    if (reply.type() == QDBusMessage::ReplyMessage && !reply.arguments().isEmpty())
        return reply.arguments().first().toUInt();
    return 0;
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
