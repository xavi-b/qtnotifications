#ifndef QNOTIFICATIONS_H
#define QNOTIFICATIONS_H

#include <QtNotifications/qnotifications_global.h>
#include <QtCore/qobject.h>
#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

class QPlatformNotificationEngine;

class Q_NOTIFICATIONS_EXPORT QNotifications : public QObject
{
    Q_OBJECT

public:
    explicit QNotifications(QObject *parent = nullptr);
    ~QNotifications();

    enum ClosedReason {
        Expired,
        Dismissed,
        Closed,
        Undefined
    };
    Q_ENUM(ClosedReason)

    bool isSupported() const;

    uint sendNotification(const QString &title,
                         const QString &message,
                         const QVariantMap &parameters = {},
                         const QMap<QString, QString> &actions = {});

Q_SIGNALS:
    void actionInvoked(uint notificationId, const QString &actionKey);
    void notificationClosed(uint notificationId, ClosedReason reason);
    void notificationClicked(uint notificationId);

private:
    Q_DISABLE_COPY(QNotifications)
    QPlatformNotificationEngine *m_engine;
};

QT_END_NAMESPACE

#endif // QNOTIFICATIONS_H
