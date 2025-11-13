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

    void handleActionInvoked(const QString &notificationIdentifier, const QString &actionKey);
    void handleNotificationClosed(const QString &notificationIdentifier, QNotifications::ClosedReason reason);
    void handleNotificationClicked(const QString &notificationIdentifier);

    bool isSupported() const override;
    uint sendNotification(const QString &title,
                         const QString &message,
                         const QVariantMap &parameters,
                         const QMap<QString, QString> &actions) override;

private:
    DarwinNotificationDelegate* m_delegate;
    // Map from notification identifier (UUID string) to notification ID
    QHash<QString, uint> m_notificationIdMap;
};

QPlatformNotificationEngine *qt_create_notification_engine_darwin();

QT_END_NAMESPACE

#endif // QPLATFORMNOTIFICATIONENGINE_DARWIN_H