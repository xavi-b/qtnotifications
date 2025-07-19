#ifndef QNOTIFICATIONS_H
#define QNOTIFICATIONS_H

#include <QtNotifications/qnotifications_global.h>
#include <QtNotifications/qplatformnotificationengine.h>
#include <QtCore/qobject.h>
#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE


class Q_NOTIFICATIONS_EXPORT QNotifications : public QObject
{
    Q_OBJECT

public:
    explicit QNotifications(QObject *parent = nullptr);
    ~QNotifications();

    enum NotificationType {
        Information,
        Warning,
        Error,
        Success
    };
    Q_ENUM(NotificationType)

    bool isSupported() const;

    bool sendNotification(const QString &title,
                         const QString &message,
                         NotificationType type = Information);

    bool sendNotification(const QString &title,
                         const QString &message,
                         const QString &iconPath,
                         NotificationType type = Information);
    bool sendNotification(const QString &title,
                         const QString &message,
                         const QMap<QString, QString> &actions,
                         NotificationType type = Information);
    bool sendNotification(const QString &title,
                         const QString &message,
                         const QString &iconPath,
                         const QMap<QString, QString> &actions,
                         NotificationType type = Information);

Q_SIGNALS:
    void actionInvoked(uint notificationId, const QString &actionKey);
    void notificationClosed();

private:
    Q_DISABLE_COPY(QNotifications)
    QPlatformNotificationEngine *m_engine;
};

QT_END_NAMESPACE

#endif // QNOTIFICATIONS_H
