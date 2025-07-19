#ifndef QPLATFORMNOTIFICATIONENGINE_H
#define QPLATFORMNOTIFICATIONENGINE_H

#include <QtNotifications/qnotifications.h>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QMap>

QT_BEGIN_NAMESPACE

class QPlatformNotificationEngine : public QObject
{
    Q_OBJECT
public:
    explicit QPlatformNotificationEngine(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~QPlatformNotificationEngine() = default;

    virtual bool isSupported() const = 0;
    virtual uint sendNotification(const QString &title,
                                 const QString &message,
                                 const QVariantMap &parameters,
                                 const QMap<QString, QString> &actions) = 0;

signals:
    void actionInvoked(uint notificationId, const QString &actionKey);
    void notificationClosed(uint notificationId, QNotifications::ClosedReason reason);
    void notificationClicked(uint notificationId);
};

QPlatformNotificationEngine *qt_notification_engine();

QT_END_NAMESPACE

#endif // QPLATFORMNOTIFICATIONENGINE_H
