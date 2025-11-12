#ifndef QPLATFORMNOTIFICATIONENGINE_LINUX_H
#define QPLATFORMNOTIFICATIONENGINE_LINUX_H

#include <QtNotifications/qplatformnotificationengine.h>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QMap>

QT_BEGIN_NAMESPACE

class QPlatformNotificationEngineLinux : public QPlatformNotificationEngine
{
    Q_OBJECT
public:
    explicit QPlatformNotificationEngineLinux(QObject *parent = nullptr);
    ~QPlatformNotificationEngineLinux() = default;

    bool isSupported() const override;
    bool sendNotification(const QString &title,
                         const QString &message,
                         const QString &iconPath,
                         const QMap<QString, QString> &actions,
                         QNotifications::NotificationType type) override;

private Q_SLOTS:
    void onActionInvoked(uint id, const QString &actionKey);
    void onNotificationClosed(uint id, uint reason);
};

QPlatformNotificationEngine *qt_create_notification_engine_linux();

QT_END_NAMESPACE

#endif // QPLATFORMNOTIFICATIONENGINE_LINUX_H