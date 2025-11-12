#ifndef QPLATFORMNOTIFICATIONENGINE_DARWIN_H
#define QPLATFORMNOTIFICATIONENGINE_DARWIN_H

#include <QtNotifications/qplatformnotificationengine.h>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QHash>


QT_BEGIN_NAMESPACE

class QPlatformNotificationEngineDarwin;

Q_FORWARD_DECLARE_OBJC_CLASS(DarwinNotificationDelegate);

class QPlatformNotificationEngineDarwin : public QPlatformNotificationEngine
{
    Q_OBJECT
public:
    explicit QPlatformNotificationEngineDarwin(QObject *parent = nullptr);
    ~QPlatformNotificationEngineDarwin();

    void handleActionInvoked(uint notificationId, const QString &actionKey);
    void handleNotificationClosed(uint notificationId, uint reason);
    void handleNotificationClicked(uint notificationId);

    bool isSupported() const override;
    bool sendNotification(const QString &title,
                         const QString &message,
                         const QString &iconPath,
                         const QMap<QString, QString> &actions,
                         QNotifications::NotificationType type) override;

private:
    DarwinNotificationDelegate* m_delegate;
};

QPlatformNotificationEngine *qt_create_notification_engine_darwin();

QT_END_NAMESPACE

#endif // QPLATFORMNOTIFICATIONENGINE_DARWIN_H